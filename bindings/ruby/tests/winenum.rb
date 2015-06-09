# winenum.rb
#  enumerate any WMI class in any namespace
#
# Usage
#   winenum [--debug] [--namespace <namespace>] <classname> [ <property> ... ]
#
# Example
# (-> http://www.codeproject.com/Articles/46390/WMI-Query-Language-by-Example)
# 1. Enumerate namespaces (call recursively!)
#   ruby winenum.rb -n root __NAMESPACE
# 2. Enumerate classnames
#   ruby winenum.rb -n root/cimv2 -q "select * from meta_class"
# 3. Get class properties
#   ruby winenum.rb -n root/cimv2 -q "select * from meta_class where __Class = \"Win32_LogicalDisk\""
# 4. Get all base classes
#   ruby winenum.rb -n root/cimv2  -q "select * from meta_class where __SuperClass Is Null"
# 5. Get immediate children of a class
#   ruby winenum.rb -n root/cimv2  -q "select * from meta_class where __SuperClass = \"CIM_Setting\""
# 6. Get the dynasty of children starting at a class
#   ruby winenum.rb -n root/cimv2  -q "select * from meta_class where __Dynasty = \"CIM_Setting\""
#
# Additional WQL parameters
# - limit to specific names
#  Where __Class Like "Win32%"
# - limit to specific origins ("is a")
#  Where __This Isa "__Event"
#
#
# Associations
# winrm enumerate wmicimv2/* -dialect:association -associations -filter:{object=win32_service?name=winrm;resultclassname=win32_dependentservice;role=dependent}
# (a: addressing, b: cimbinding, w: dmtf wsman, p: microsoft wsman)
# <n:Enumerate>
#   <w:OptimizeEnumeration/>
#   <w:MaxElements>32000</w:MaxElements>
#   <w:Filter Dialect="http://schemas.dmtf.org/wbem/wsman/1/cimbinding/associationFilter">
#     <b:AssociationInstances>
#       <b:Object>
#         <a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address>
#         <a:ReferenceParameters>
#           <w:ResourceURI>http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/win32_service</w:ResourceURI>
#           <w:SelectorSet>
#             <w:Selector Name="name">winrm</w:Selector>
#           </w:SelectorSet>
#         </a:ReferenceParameters>
#       </b:Object>
#       <b:ResultClassName>win32_dependentservice</b:ResultClassName>
#       <b:Role>dependent</b:Role>
#     </b:AssociationInstances>
#   </w:Filter>
# </n:Enumerate>
#
# winrm enumerate wmicimv2/* -dialect:association -associations -filter:{object=win32_service?name=winrm}
# <n:Enumerate>
#   <w:OptimizeEnumeration/>
#   <w:MaxElements>32000</w:MaxElements>
#   <w:Filter Dialect="http://schemas.dmtf.org/wbem/wsman/1/cimbinding/associationFilter">
#     <b:AssociationInstances>
#       <b:Object>
#         <a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address>
#         <a:ReferenceParameters>
#           <w:ResourceURI>http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/win32_service</w:ResourceURI>
#           <w:SelectorSet>
#             <w:Selector Name="name">winrm</w:Selector>
#           </w:SelectorSet>
#         </a:ReferenceParameters>
#       </b:Object>
#     </b:AssociationInstances>
#   </w:Filter>
# </n:Enumerate></s:Body></s:Envelope>


require 'rexml/document'
require File.expand_path(File.join(File.dirname(__FILE__),'_loadpath'))
require 'openwsman'
require '_client'
require 'getoptlong'

WIDTH = 25

#
# parse classname and selectors from string
#
# <classname>?key=value&key=value...
#

def parse_classname str
  raise "Classname required" unless str
  classname, args = str.split "?"
  return [classname] unless args
  selectors = {}
  args.split("&").each do |kv|
    key,value = kv.split "="
    selectors[key] = value
  end
  [classname,selectors]
end

#
# extract reference information from a node
#
# returns a [ <uri>, <selectors> ] pair
#
def get_reference_from node
  uri = node.ReferenceParameters.ResourceURI
  selectors = {}
  node.ReferenceParameters.SelectorSet.each do |s|
    name = s.attr_find(nil, "Name")
    selectors[name] = s.text
  end
  [uri,selectors]
end

#
# Extract filter arguments
# --{associators,references} <ResultClassName>,<Role>[,<ResultRole>]
# e.g. Win32_DependentService,dependent
# e.g. Win32_Service,dependent,antecedent
#
# @returns [AssociationClassName, result_class, role, result_role ]
#
def extract_filter_arguments arg
  if arg.nil? || arg.empty?
    return nil
  end
  a,b,c,d = arg.split ","
  a = nil if a && a.empty?
  b = nil if b && b.empty?
  c = nil if c && c.empty?
  d = nil if d && d.empty?
  [a,b,c,d]
end

def print_item indent, item, properties = []
  indentation = "  "*indent
  if properties.empty?
    #      puts node.string
    item.each do |c|
      if c.size == 0
        attrs = ""
        c.each_attr { |a| attrs << " #{a.name}=#{a.value}" }
        attrs = "<"+attrs+">" unless attrs.empty?
        puts "#{indentation}#{c.name}#{attrs}: #{c.text}"
      else
        print_item indent+1, c
      end
    end
  else
    values = []
    properties.each do |p|
      values << item.send(p)
    end
    printf "#{indentation}%-#{WIDTH}s\n" * values.size, *values
  end
end

#
# Enumerate
#
# Enumerates instances of a class
#   parms[:epr] - end point references only (aka 'names')
#   parms[:associators] - enumerate associators
#   parms[:references] - enumerate references
#

def enum_properties client, parms, *properties
  options = Openwsman::ClientOptions.new
  namespace = parms[:namespace] || "root/cimv2"
  classname = parms[:classname]
  limit = parms[:limit].to_i rescue 0
  faults = 0

  filter = nil
  if parms[:query]
    q = parms[:query]
    filter = Openwsman::Filter.new
    filter.wql q
    classname = "*"
    if q =~ /.*from(\s+)(([\w_])+)/i
      uri = Openwsman.epr_prefix_for $2, namespace
      uri << "/*"
    end
  end

  start_time = Time.new

#  Openwsman::debug = -1  
#  Openwsman.debug = -1  
#  options.max_envelope_size = 1024 * 1024 * 1024
#  puts "max_envelope_size #{options.max_envelope_size}"
  options.set_dump_request if parms[:debug]

  # timeout
  if parms[:timeout]
    options.timeout = parms[:timeout].to_i * 1000 # parms is in sec, timeout in msec
  end

  # locale
  if parms[:locale]
    options.locale = parms[:locale]
  end

  options.flags = Openwsman::FLAG_ENUMERATION_OPTIMIZATION
  options.max_elements = 999
  
  # return endpoint references (instead of instances)
  options.flags = Openwsman::FLAG_ENUMERATION_ENUM_EPR if parms[:epr]

  uri ||= Openwsman.epr_uri_for namespace, classname

  # enumerate Associators
  if parms[:associators] || parms[:references]
#    options.flags = Openwsman::FLAG_ENUMERATION_ENUM_OBJ_AND_EPR
    filter = Openwsman::Filter.new
    selectors = parms[:selectors]
    epr = Openwsman::EndPointReference.new(uri, nil, selectors)
    if parms[:associators]
      assoc_class, result_class, role, result_role = extract_filter_arguments parms[:associators]
      filter.associators(epr, assoc_class, result_class, role, result_role)
    elsif parms[:references]
      assoc_class, result_class, role, result_role = extract_filter_arguments parms[:references]
      filter.references(epr, assoc_class, result_class, role, result_role)
    end
    classname = "*" # accept any result classname
    uri = Openwsman.epr_uri_for "", classname
  end
  # enumerate References
  
#  STDERR.puts "URI <#{uri}>"
  result = client.enumerate( options, filter, uri )
  
  unless properties.empty?
    printf "%-#{WIDTH}s" * properties.size, *properties
    puts
    puts "-" * WIDTH * properties.size
  end
  results = 0
  context = nil
 
  loop do
    break if fault? client, result
    puts "result #{result.to_xml}" if parms[:debug]

    items = result.Items
    break unless items
    items.each do |item|
      if parms[:epr]
        epr = Openwsman::EndPointReference.new(item)
        puts epr
        results += 1
      else
      #    puts items.to_xml if parms[:debug]
        puts "-------" if results > 0
        unless classname == "*"
          next unless item.name == classname
        end
        puts item.name
        results += 1
        print_item 1, item, properties
      end
    end # Items.each
    limit -= 1
    break if limit == 0

    context = result.context
    break unless context
    result = client.pull( options, nil, uri, context )

    puts
  end

  client.release( options, uri, context ) if context
  duration = Time.new - start_time
  printf "%d results, %d faults in %3.2f seconds\n" % [results, faults, duration]
end

def usage msg=nil
  STDERR.puts msg if msg
  STDERR.puts "Usage:"
  STDERR.puts "winenum [-n <namespace>] [-q <wql-query>] [-l <limit>] [-d] <class>"
  STDERR.puts "\twith -q <query>, <class> is discarded"
  STDERR.puts "\nand remember to set WSMANCLIENT"
  exit 1
end
  
#
# --- main ---
#
#

client = Client.open

parms = {}

begin
  opts = GetoptLong.new(
           [ "--associators", "-a", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--debug", "-d", GetoptLong::NO_ARGUMENT ],
           [ "--epr", "-e", GetoptLong::NO_ARGUMENT ],
           [ "--limit", "-l", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--namespace", "-n", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--query", "-q", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--references", "-r", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--timeout", "-t", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--locale", "-L", GetoptLong::REQUIRED_ARGUMENT ]
         )
  opts.each do |opt,arg|
    case opt
    when "--query" then parms[:query] = arg
    when "--limit" then parms[:limit] = arg
    when "--namespace" then parms[:namespace] = arg
    when "--debug" then parms[:debug] = true
    when "--epr" then parms[:epr] = true
    when "--associators" then parms[:associators] = arg
    when "--references" then parms[:references] = arg
    when "--timeout" then parms[:timeout] = arg
    when "--locale" then parms[:locale] = arg
    end
  end
rescue GetoptLong::InvalidOption
  usage "invalid option passed"
end

if parms[:associators] && parms[:references]
  usage "Enumeration of Associators and References not supported"
end

unless parms[:query]
  parms[:classname], parms[:selectors] = parse_classname(ARGV.shift)
end

#puts "Classname(#{parms[:classname]}) Selectors(#{parms[:selectors].inspect})"
enum_properties client, parms, *ARGV
