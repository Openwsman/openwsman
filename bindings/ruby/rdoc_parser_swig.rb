
require 'rdoc/parser/ruby'
require 'rdoc/known_classes'

##
# RDoc::Parser::SWIG attempts to parse SWIG extension files.  It looks for
# the standard patterns that you find in extensions: <tt>%extend, %define,
# %rename, %alias</tt> and so on.  It tries to find the corresponding
# C source for the methods and extract comments, but if we fail
# we don't worry too much.
#
# The comments associated with a Ruby method are extracted from the SWIG
# comment block associated with the routine that _implements_ that
# method
#
# The comment blocks may include special directives:
#
# [Document-class: +name+]
#   Documentation for the named class.
#
# [Document-module: +name+]
#   Documentation for the named module.
#
# [Document-const: +name+]
#   Documentation for the named +rb_define_const+.
#
# [Document-global: +name+]
#   Documentation for the named +rb_define_global_const+
#
# [Document-variable: +name+]
#   Documentation for the named +rb_define_variable+
#
# [Document-method: +method_name+]
#   Documentation for the named method.  Use this when the method name is
#   unambiguous.
#
# [Document-method: <tt>ClassName::method_name<tt>]
#   Documentation for a singleton method in the given class.  Use this when
#   the method name alone is ambiguous.
#
# [Document-method: <tt>ClassName#method_name<tt>]
#   Documentation for a instance method in the given class.  Use this when the
#   method name alone is ambiguous.
#
# [Document-attr: +name+]
#   Documentation for the named attribute.
#
# [call-seq:  <i>text up to an empty line</i>]
#   Because C source doesn't give descriptive names to Ruby-level parameters,
#   you need to document the calling sequence explicitly
#
#
# As an example, we might have an extension that defines multiple classes
# in its Init_xxx method. We could document them using
#
#   /*
#    * Document-class:  MyClass
#    *
#    * Encapsulate the writing and reading of the configuration
#    * file. ...
#    */
#
#   /*
#    * Document-method: read_value
#    *
#    * call-seq:
#    *   cfg.read_value(key)            -> value
#    *   cfg.read_value(key} { |key| }  -> value
#    *
#    * Return the value corresponding to +key+ from the configuration.
#    * In the second form, if the key isn't found, invoke the
#    * block and return its value.
#    */

class RDoc::Parser::SWIG < RDoc::Parser

  parse_files_matching(/\.i\z/)

  include RDoc::Text

  ##
  # SWIG file the parser is parsing

  attr_accessor :content


  ##
  # Maps C type names to names of ruby classes (andsingleton classes)

  attr_reader :known_classes

  ##
  # Maps C type names to names of ruby singleton classes

  attr_reader :singleton_classes

  ##
  # Resets cross-file state.  Call when parsing different projects that need
  # separate documentation.

  def self.reset
    @@enclosure_classes = {}
    @@known_bodies = {}
  end

  reset

  ##
  # Prepare to parse a SWIG file

  def initialize(top_level, file_name, content, options, stats)
    super

    @known_classes = RDoc::KNOWN_CLASSES.dup
    @content = handle_tab_width handle_ifdefs_in(@content)
    @renames = {} # maps old_name => [ new_name, args ]
    @aliases = {} # maps name => [ alias_name, args ]
    @classes = {}
    @singleton_classes = {}
    @file_dir = File.dirname(@file_name)
  end

  ##
  # Scans #content for %alias

  def do_aliases
    @content.scan(/%alias\s+(\w+)\s+"([^"]+)";/) do |old_name, new_name| # "
#      class_name = @known_classes[var_name]
#
#      unless class_name then
#        warn "Enclosing class/module %p for alias %s %s not known" % [
#          var_name, new_name, old_name]
#        next
#      end
#
#      class_obj = find_class var_name, class_name

      al = RDoc::Alias.new '', old_name, new_name, ''
#      al.singleton = @singleton_classes.key? var_name

#      comment = find_alias_comment var_name, new_name, old_name
#      comment = strip_stars comment
#      al.comment = comment

      al.record_location @top_level

#      class_obj.add_alias al
      @stats.add_alias al
    end
  end

  ##
  # Scans #content for rb_attr and rb_define_attr

  def do_attrs
    @content.scan(/rb_attr\s*\(
                   \s*(\w+),
                   \s*([\w"()]+),
                   \s*([01]),
                   \s*([01]),
                   \s*\w+\);/xm) do |var_name, attr_name, read, write|
      handle_attr var_name, attr_name, read, write
    end

    @content.scan(%r%rb_define_attr\(
                             \s*([\w\.]+),
                             \s*"([^"]+)",
                             \s*(\d+),
                             \s*(\d+)\s*\);
                %xm) do |var_name, attr_name, read, write|
      handle_attr var_name, attr_name, read, write
    end
  end

  ##
  # Scans #content for %rename

  def do_renames
    @content.scan(/\s*%rename\s*\(\s*"?([\w_=]+)"?\s*\)\s*([\w_]+)\s*\(?([\w_,\*\s]+)?\s*\)?;/) do
      |new_name, old_name, args|
      @renames[old_name] = [new_name, args]
    end
  end

  ##
  # Scans #content for first(!) %module

  def do_modules    
    @content.scan(/%module\s+(\w+)/) do |match_data|
      module_name = match_data[0]
      @module = handle_class_module(nil, "module", module_name, @toplevel)
      break # first %module only
    end
    @@module ||= @module
    @module = @@module
  end

  ##
  # Scans #content for %extend

  def do_classes content
    found = false
#    puts "do_classes #{content.inspect}"
    content.scan(/%extend\s+([\w_]+)(.*)/m) do |class_name, content|
      real_name = @renames[class_name][0] rescue nil
      klass = handle_class_module(class_name, "class", real_name, @module)
      # now check if we have multiple %extend, the regexp above is greedy and will match all of them
      while content =~ /%extend/
        do_classes $& + $' # ' add the %extend back in
        content = $` # real content is everything before the embedded %extend
      end
#      puts "%extend #{class_name}: #{content.inspect}"
      do_methods klass, class_name, content
      found = true
    end
    found
  end

  ##
  # Scans #content for #define and %constant

  def do_constants
    # %constant <type>[*] <name> = <c-constant>;

    @content.scan(/(\/\*.*?\*\/)?\s*%constant\s+([\w_]+)([\s\*]+)([\w_]+)\s*=\s*([\w_]+);/) do |comment, type, pointer, const_name, definition|
#      puts "\nConst #{const_name} : #{comment.inspect}"
      handle_constants comment, const_name, definition
    end
  end

  ##
  # Scans #content for %mixin

  def do_includes
    @content.scan(/%mixin/) do |c,m|
      if cls = @classes[c]
        m = @known_classes[m] || m
        incl = cls.add_include RDoc::Include.new(m, "")
        incl.record_location @top_level
      end
    end
  end

  ##
  # Scans content for methods
  # klass is a RDoc::NormalClass
  # name is the 'C' (type) name
  # content is what's enclosed in %extend <name> { ... }

  def do_methods klass, name, content
    # Find class constructor as 'new'
    content.scan(/^\s+#{name}\s*\(([^\)]*)\)\s*\{/m) do |args|
      handle_method("method", klass.name, "initialize", name, (args.to_s.split(",")||[]).size, content)
    end
      content.scan(%r{static\s+((const\s+)?\w+)([\s\*]+)(\w+)\s*\(([^\)]*)\)\s*;}) do
        |const,type,pointer,meth_name,args|
        handle_method("method", klass.name, meth_name, nil, (args.split(",")||[]).size, content)
      end
      content.scan(/((const\s+)?\w+)  # <const>? <type>
			([ \t\*]+)    # <pointer>?
			(\w+)	      # <meth>
			\s*\(([^\)]*)\) # args
			\s*\{/xm) do  # function def start
        |const,type,pointer,meth_name,args|
#          puts "do_methods klass #{klass}, name #{name}: const #{const.inspect}, type #{type.inspect}, pointer #{pointer.inspect}, meth_name #{meth_name.inspect}"
	next unless meth_name
	next if meth_name =~ /~/
	type = "string" if type =~ /char/ && pointer =~ /\*/
        handle_method("method", klass.name, meth_name, nil, (args.split(",")||[]).size, content)
      end
  end

  ##
  # Finds the comment for an alias on +class_name+ from +new_name+ to
  # +old_name+

  def find_alias_comment class_name, new_name, old_name
    content =~ %r%((?>/\*.*?\*/\s+))
                  rb_define_alias\(\s*#{Regexp.escape class_name}\s*,
                                   \s*"#{Regexp.escape new_name}"\s*,
                                   \s*"#{Regexp.escape old_name}"\s*\);%xm

    $1 || ''
  end

  ##
  # Finds a comment for rb_define_attr, rb_attr or Document-attr.
  #
  # +var_name+ is the C class variable the attribute is defined on.
  # +attr_name+ is the attribute's name.
  #
  # +read+ and +write+ are the read/write flags ('1' or '0').  Either both or
  # neither must be provided.

  def find_attr_comment var_name, attr_name, read = nil, write = nil
    attr_name = Regexp.escape attr_name

    rw = if read and write then
           /\s*#{read}\s*,\s*#{write}\s*/xm
         else
           /.*?/m
         end

    if @content =~ %r%((?>/\*.*?\*/\s+))
                      rb_define_attr\((?:\s*#{var_name},)?\s*
                                      "#{attr_name}"\s*,
                                      #{rw}\)\s*;%xm then
      $1
    elsif @content =~ %r%((?>/\*.*?\*/\s+))
                         rb_attr\(\s*#{var_name}\s*,
                                  \s*#{attr_name}\s*,
                                  #{rw},.*?\)\s*;%xm then
      $1
    elsif @content =~ %r%Document-attr:\s#{attr_name}\s*?\n
                         ((?>.*?\*/))%xm then
      $1
    else
      ''
    end
  end

  ##
  # Find the C code corresponding to a Ruby method

  def find_body class_name, meth_name, meth_obj, file_content, quiet = false
# puts "\n\tfind_body #{meth_obj.c_function.inspect}[#{meth_name.inspect}]" if meth_obj.name == "new"
# puts "\n\tfind_body #{meth_name}[#{meth_obj.name}] ?"
# puts "\n\t#{file_content}"
    case file_content
#    when %r%(/\*.*\*/\s*#{meth_obj.c_function})%xm
    when %r%((?>/\*.*?\*/\s+)?)
            ((?:(?:static\s*)?(?:\s*const)?(?:\s*unsigned)?(?:\s*struct)?\s+)?
             (VALUE|[\w_]+)(\s+\*|\*\s+|\s+)#{meth_name}
             \s*(\([^)]*\))([^;]|$))%xm,
         %r%(/\*.*?\*/\s+)
             #{meth_name}
             \s*(\([^)]*\))([^;]|$)%xm then
# puts "\n  found! [1:#{$1.inspect},2:#{$2.inspect},3:#{$3.inspect},#{$4.inspect},#{$5.inspect},#{$6.inspect}]" if meth_obj.name == "detail"

      comment = $1
      body = $2
      # type = $3
      return false if $3 == "define" # filter out SWIG_Exception
      return false if $3.nil? && meth_name != "initialize" # constructor has no type
      # ptr = $4
      offset = $~.offset(2).first

      remove_private_comments comment if comment

      # try to find the whole body
      body = $& if /#{Regexp.escape body}[^(]*?\{.*?^\}/m =~ file_content

# puts "\n\tfind_body #{meth_name} -> #{body}"
      # The comment block may have been overridden with a 'Document-method'
      # block. This happens in the interpreter when multiple methods are
      # vectored through to the same C method but those methods are logically
      # distinct (for example Kernel.hash and Kernel.object_id share the same
      # implementation

      override_comment = find_override_comment class_name, meth_obj
      comment = override_comment if override_comment

      find_modifiers comment, meth_obj if comment

      #meth_obj.params = params
      meth_obj.start_collecting_tokens
      tk = RDoc::RubyToken::Token.new nil, 1, 1
      tk.set_text body
      meth_obj.add_token tk
      meth_obj.comment = strip_stars comment
      meth_obj.offset  = offset
      meth_obj.line    = file_content[0, offset].count("\n") + 1

      body
    when %r%((?>/\*.*?\*/\s*))^\s*(\#\s*define\s+#{meth_name}\s+(\w+))%m then
      comment = $1
      body = $2
      offset = $~.offset(2).first

      find_body class_name, $3, meth_obj, file_content, true
      find_modifiers comment, meth_obj

      meth_obj.start_collecting_tokens
      tk = RDoc::RubyToken::Token.new nil, 1, 1
      tk.set_text body
      meth_obj.add_token tk
      meth_obj.comment = strip_stars(comment) + meth_obj.comment.to_s
      meth_obj.offset  = offset
      meth_obj.line    = file_content[0, offset].count("\n") + 1

      body
    when %r%^\s*\#\s*define\s+#{meth_name}\s+(\w+)%m then
      # with no comment we hope the aliased definition has it and use it's
      # definition

      body = find_body(class_name, $1, meth_obj, file_content, true)

      return body if body

      warn "No definition for #{meth_name}" if @options.verbosity > 1
      false
    else # No body, but might still have an override comment
      comment = find_override_comment class_name, meth_obj

      if comment then
        find_modifiers comment, meth_obj
        meth_obj.comment = strip_stars comment

        ''
      else
        warn "No definition for #{meth_name}" if @options.verbosity > 1
        false
      end
    end
  end

  ##
  # Finds a RDoc::NormalClass or RDoc::NormalModule for +raw_name+

  def find_class(raw_name, name)
    unless @classes[raw_name]
      if raw_name =~ /^rb_m/
        container = @top_level.add_module RDoc::NormalModule, name
      else
        container = @top_level.add_class RDoc::NormalClass, name
      end

      container.record_location @top_level
      @classes[raw_name] = container
    end
    @classes[raw_name]
  end

  ##
  # Look for class or module documentation above Init_+class_name+(void),
  # in a Document-class +class_name+ (or module) comment or above an
  # rb_define_class (or module).  If a comment is supplied above a matching
  # Init_ and a rb_define_class the Init_ comment is used.
  #
  #   /*
  #    * This is a comment for Foo
  #    */
  #   Init_Foo(void) {
  #       VALUE cFoo = rb_define_class("Foo", rb_cObject);
  #   }
  #
  #   /*
  #    * Document-class: Foo
  #    * This is a comment for Foo
  #    */
  #   Init_foo(void) {
  #       VALUE cFoo = rb_define_class("Foo", rb_cObject);
  #   }
  #
  #   /*
  #    * This is a comment for Foo
  #    */
  #   VALUE cFoo = rb_define_class("Foo", rb_cObject);

  def find_class_comment(class_name, class_mod)
    comment = nil

    if @content =~ %r%Document-(?:class|module):\s+#{class_name}\s*?
                         (?:<\s+[:,\w]+)?\n((?>.*?\*/))%xm then
      comment = $1
    elsif @content =~ %r%
        ((?>/\*.*?\*/\s+))
        (static\s+)?
        void\s+
        Init_#{class_name}\s*(?:_\(\s*)?\(\s*(?:void\s*)?\)%xmi then
      comment = $1.sub(%r%Document-(?:class|module):\s+#{class_name}%, '')
    end

    return unless comment

    comment = strip_stars comment

    comment = look_for_directives_in class_mod, comment

    class_mod.add_comment comment, @top_level
  end

  ##
  # Handles modifiers in +comment+ and updates +meth_obj+ as appropriate.
  #
  # If <tt>:nodoc:</tt> is found, documentation on +meth_obj+ is suppressed.
  #
  # If <tt>:yields:</tt> is followed by an argument list it is used for the
  # #block_params of +meth_obj+.
  #
  # If the comment block contains a <tt>call-seq:</tt> section like:
  #
  #   call-seq:
  #      ARGF.readlines(sep=$/)     -> array
  #      ARGF.readlines(limit)      -> array
  #      ARGF.readlines(sep, limit) -> array
  #
  #      ARGF.to_a(sep=$/)     -> array
  #      ARGF.to_a(limit)      -> array
  #      ARGF.to_a(sep, limit) -> array
  #
  # it is used for the parameters of +meth_obj+.

  def find_modifiers comment, meth_obj
    # we must handle situations like the above followed by an unindented first
    # comment.  The difficulty is to make sure not to match lines starting
    # with ARGF at the same indent, but that are after the first description
    # paragraph.

    if comment =~ /call-seq:(.*?(?:\S|\*\/?).*?)^\s*(?:\*\/?)?\s*$/m then
      all_start, all_stop = $~.offset(0)
      seq_start, seq_stop = $~.offset(1)

      # we get the following lines that start with the leading word at the
      # same indent, even if they have blank lines before
      if $1 =~ /(^\s*\*?\s*\n)+^(\s*\*?\s*\w+)/m then
        leading = $2 # ' *    ARGF' in the example above
        re = %r%
          \A(
             (^\s*\*?\s*\n)+
             (^#{Regexp.escape leading}.*?\n)+
            )+
          ^\s*\*?\s*$
        %xm
        if comment[seq_stop..-1] =~ re then
          all_stop = seq_stop + $~.offset(0).last
          seq_stop = seq_stop + $~.offset(1).last
        end
      end

      seq = comment[seq_start..seq_stop]
      seq.gsub!(/^(\s*\*?\s*?)(\S|\n)/m, '\2')
      comment.slice! all_start...all_stop
      meth_obj.call_seq = seq
    elsif comment.sub!(/\A\/\*\s*call-seq:(.*?)\*\/\Z/, '') then
      meth_obj.call_seq = $1.strip
    end

    look_for_directives_in meth_obj, comment
  end

  ##
  # Finds a <tt>Document-method</tt> override for +meth_obj+ on +class_name+

  def find_override_comment class_name, meth_obj
    name = Regexp.escape meth_obj.name
    prefix = Regexp.escape meth_obj.name_prefix

    if @content =~ %r%Document-method:\s+#{class_name}#{prefix}#{name}\s*?\n((?>.*?\*/))%m then
      $1
    elsif @content =~ %r%Document-method:\s#{name}\s*?\n((?>.*?\*/))%m then
      $1
    end
  end

  ##
  # Creates a new RDoc::Attr +attr_name+ on class +var_name+ that is either
  # +read+, +write+ or both

  def handle_attr(var_name, attr_name, read, write)
    rw = ''
    rw << 'R' if '1' == read
    rw << 'W' if '1' == write

    class_name = @known_classes[var_name]

    return unless class_name

    class_obj = find_class var_name, class_name

    return unless class_obj

    comment = find_attr_comment var_name, attr_name
    comment = strip_stars comment

    name = attr_name.gsub(/rb_intern\("([^"]+)"\)/, '\1') # "

    attr = RDoc::Attr.new '', name, rw, comment

    attr.record_location @top_level
    class_obj.add_attribute attr
    @stats.add_attribute attr
  end

  ##
  # Creates a new RDoc::NormalClass or RDoc::NormalModule based on +type+
  # named +class_name+ in +parent+ which was assigned to the C +var_name+.

  def handle_class_module(type_name, type, class_name, enclosure)
    enclosure ||= @top_level

    if type == "class" then
      full_name = if RDoc::ClassModule === enclosure then
                    enclosure.full_name + "::#{class_name}"
                  else
                    class_name
                  end

      if @content =~ %r%Document-class:\s+#{full_name}\s*<\s+([:,\w]+)% then
        parent_name = $1
      end

      cm = enclosure.add_class RDoc::NormalClass, class_name, parent_name
    else
      cm = enclosure.add_module RDoc::NormalModule, class_name
    end

    cm.record_location enclosure.top_level

    find_class_comment class_name, cm

    case cm
    when RDoc::NormalClass
      @stats.add_class cm
    when RDoc::NormalModule
      @stats.add_module cm
    end

    @classes[type_name] = cm
    @@enclosure_classes[class_name] = cm
    @known_classes[class_name] = cm.full_name
    cm
  end

  ##
  # Adds constants.
  #

  def handle_constants(comment, const_name, definition)
    class_obj = @module

    unless class_obj then
      warn "Enclosing class/module #{const_name.inspect} not known"
      return
    end

    class_name = class_obj.name

    if comment
      comment = strip_stars comment
      comment = normalize_comment comment
    else
      comment = ""
    end

    con = RDoc::Constant.new const_name, definition, comment
    con.record_location @top_level
    @stats.add_constant con
    class_obj.add_constant con
  end

  ##
  # Removes #ifdefs that would otherwise confuse us

  def handle_ifdefs_in(body)
    result = ""
    loop do
      body.match(/^#if ((\s*\|\|\s*)?defined\(SWIG([\w]+)\))+/) do
        result << $` # copy everything before #if
        after = $' #'
        if $1 =~ /RUBY/ # if defined(SWIGRUBY)
          raise "Unmatched #if SWIGRUBY\n#{after}" unless after.match /^#(if|else|endif)/
          result << $` # copy everything between SWIGRUBY and #if/else/endif
          case $1
          when "if"
            result << handle_ifdefs_in($& + $') #'
          when "else"
            after = $' #'
            raise "Unclosed #else" unless after.match /^#endif/
            result << handle_ifdefs_in($') #'
          when "endif"
            result << handle_ifdefs_in($') #'
          end
        else # if defined(SWIG...)
          # throw away everything between SIWG... and #if/else/endif
          raise "Unmatched #if" unless after.match /^#(if|else|endif)/
          case $1
          when "if"
            result << handle_ifdefs_in($& + $') #'
          when "else"
            after = $' #'
            raise "Unclosed #else" unless after.match /^#endif/
            result << $` # copy everything between #else and #endif
            result << handle_ifdefs_in($') #'
          when "endif"
            result << handle_ifdefs_in($') #'
          end
        end
      end
      break
    end
    result.empty? ? body : result
  end

  ##
  # Adds an RDoc::AnyMethod +meth_name+ defined on a class or module assigned
  # to +var_name+.  +type+ is the type of method definition function used.
  # +singleton_method+ and +module_function+ create a singleton method.

  def handle_method(type, klass_name, meth_name, function, param_count, content = nil,
                    source_file = nil)
    ruby_name = (@renames[meth_name][0] rescue nil) || meth_name
# puts "\n\thandle_method #{type},#{klass_name},#{meth_name}[#{ruby_name}],#{function},#{param_count} args" # if meth_name == "initialize"
    class_name = @known_classes[klass_name]
    singleton  = @singleton_classes.key? klass_name

    return unless class_name

    class_obj = find_class klass_name, class_name
# puts "\n\tclass_obj #{class_obj.inspect}" #if meth_name == "initialize"

    if class_obj then
      if meth_name == "initialize" then
        ruby_name = 'new'
        singleton = true
        type = 'method' # force public
      end

      function ||= meth_name
      meth_obj = RDoc::AnyMethod.new '', ruby_name
      meth_obj.c_function = function
      meth_obj.singleton = singleton

      p_count = Integer(param_count) rescue -1

      if source_file then
        file_name = File.join @file_dir, source_file

        if File.exist? file_name then
          file_content = (@@known_bodies[file_name] ||= File.read(file_name))
        else
          warn "unknown source #{source_file} for #{meth_name} in #{@file_name}"
        end
      else
        file_content = content || @content
      end

      body = find_body class_name, function, meth_obj, file_content
#      puts "find_body #{class_name}##{function} -> #{body}"
      if body and meth_obj.document_self then
        meth_obj.params = if p_count < -1 then # -2 is Array
                            '(*args)'
                          elsif p_count == -1 then # argc, argv
                            rb_scan_args body
                          else
                            "(#{(1..p_count).map { |i| "p#{i}" }.join ', '})"
                          end


        meth_obj.record_location @top_level
        class_obj.add_method meth_obj
        @stats.add_method meth_obj
        meth_obj.visibility = :private if 'private_method' == type
      end
    end
  end

  ##
  # Registers a singleton class +sclass_var+ as a singleton of +class_var+

  def handle_singleton sclass_var, class_var
    class_name = @known_classes[class_var]

    @known_classes[sclass_var]     = class_name
    @singleton_classes[sclass_var] = class_name
  end

  ##
  # Normalizes tabs in +body+

  def handle_tab_width(body)
    if /\t/ =~ body
      tab_width = @options.tab_width
      body.split(/\n/).map do |line|
        1 while line.gsub!(/\t+/) do
          ' ' * (tab_width * $&.length - $`.length % tab_width)
        end && $~
        line
      end.join "\n"
    else
      body
    end
  end

  ##
  # Look for directives in a normal comment block:
  #
  #   /*
  #    * :title: My Awesome Project
  #    */
  #
  # This method modifies the +comment+

  def look_for_directives_in context, comment
    @preprocess.handle comment, context do |directive, param|
      case directive
      when 'main' then
        @options.main_page = param
        ''
      when 'title' then
        @options.default_title = param if @options.respond_to? :default_title=
        ''
      end
    end

    comment
  end

  ##
  # Extracts parameters from the +method_body+ and returns a method
  # parameter string.  Follows 1.9.3dev's scan-arg-spec, see README.EXT

  def rb_scan_args method_body
    method_body =~ /rb_scan_args\((.*?)\)/m
    return '(*args)' unless $1

    $1.split(/,/)[2] =~ /"(.*?)"/ # format argument
    format = $1.split(//)

    lead = opt = trail = 0

    if format.first =~ /\d/ then
      lead = $&.to_i
      format.shift
      if format.first =~ /\d/ then
        opt = $&.to_i
        format.shift
        if format.first =~ /\d/ then
          trail = $&.to_i
          format.shift
          block_arg = true
        end
      end
    end

    if format.first == '*' and not block_arg then
      var = true
      format.shift
      if format.first =~ /\d/ then
        trail = $&.to_i
        format.shift
      end
    end

    if format.first == ':' then
      hash = true
      format.shift
    end

    if format.first == '&' then
      block = true
      format.shift
    end

    # if the format string is not empty there's a bug in the C code, ignore it

    args = []
    position = 1

    (1...(position + lead)).each do |index|
      args << "p#{index}"
    end

    position += lead

    (position...(position + opt)).each do |index|
      args << "p#{index} = v#{index}"
    end

    position += opt

    if var then
      args << '*args'
      position += 1
    end

    (position...(position + trail)).each do |index|
      args << "p#{index}"
    end

    position += trail

    if hash then
      args << "p#{position} = {}"
      position += 1
    end

    args << '&block' if block

    "(#{args.join ', '})"
  end

  ##
  # Removes private comments from +comment+

  def remove_private_comments(comment)
    comment.gsub!(/\/?\*--\n(.*?)\/?\*\+\+/m, '')
    comment.sub!(/\/?\*--\n.*/m, '')
  end

  ##
  # Extracts the classes, modules, methods, attributes, constants and aliases
  # from a SWIG file and returns an RDoc::TopLevel for this file

  def scan
    do_modules
    do_renames
    do_aliases
    do_constants
    have_classes = do_classes @content # -> do_methods
    do_methods @module, @module.name, @content unless have_classes # file without %extend
#    do_includes
#    do_attrs
    @top_level
  end

end

