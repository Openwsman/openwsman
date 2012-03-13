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


require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'
require 'getoptlong'

WIDTH = 25

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

def show_fault result
  fault = Openwsman::Fault.new result
  puts "Fault code #{fault.code}, subcode #{fault.subcode}"
  puts "\treason #{fault.reason}"
  puts "\tdetail #{fault.detail}"
end

def enum_properties client, parms, *properties
  options = Openwsman::ClientOptions.new
  namespace = parms[:namespace] || "root/cimv2"
  classname = parms[:classname]
  limit = parms[:limit].to_i rescue 0
  faults = 0

  filter = nil
  if parms[:query]
    filter = Openwsman::Filter.new
    filter.wql parms[:query]
    classname = "*"
  end


#  Openwsman::debug = -1  
#  Openwsman.debug = -1  
#  options.max_envelope_size = 1024 * 1024 * 1024
#  puts "max_envelope_size #{options.max_envelope_size}"
  options.set_dump_request if parms[:debug]

  uri = Openwsman.epr_uri_for namespace, classname
  STDERR.puts "URI <#{uri}>"
  result = client.enumerate( options, filter, uri )
  show_fault result if result.fault?

  puts "client.enumerate => #{result.to_xml}" if parms[:debug]
  puts classname
  unless properties.empty?
    printf "%-#{WIDTH}s" * properties.size, *properties
    puts
    puts "-" * WIDTH * properties.size
  end
  results = 0
  context = nil
 
  loop do
    context = result.context
    break unless context

    result = client.pull( options, nil, uri, context )
    break unless result
    puts "client.pull #{result.to_xml}" if parms[:debug]

    if result.fault?
        show_fault result
	faults += 1
	break
    end

    body = result.body
    if classname == "*"
      items = body.PullResponse.Items
    else
      items = body.PullResponse.Items.send classname
    end
    next unless items
    results += 1
#    puts items.to_xml if parms[:debug]
    puts "-------" if results > 1
    if classname == "*"
      node = items.first
      puts node.name
    else
      node = items.send classname
    end
    if properties.empty?
#      puts node.string
      node.each do |c|
	puts "  #{c.name}: #{c.text}"
      end
    else
      values = []
      properties.each do |p|
	values << node.send(p)
      end
      printf "%-#{WIDTH}s" * values.size, *values
      puts
    end
    limit -= 1
    break if limit == 0

    puts
  end

  client.release( options, uri, context ) if context
  puts "#{results} results, #{faults} faults"
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
           [ "--query", "-q", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--limit", "-l", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--namespace", "-n", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--debug", "-d", GetoptLong::NO_ARGUMENT ]
         )
  opts.each do |opt,arg|
    case opt
    when "--query" then parms[:query] = arg
    when "--limit" then parms[:limit] = arg
    when "--namespace" then parms[:namespace] = arg
    when "--debug" then parms[:debug] = true
    end
  end
rescue GetoptLong::InvalidOption
  usage "invalid option passed"
end

parms[:classname] = ARGV.shift

enum_properties client, parms, *ARGV
