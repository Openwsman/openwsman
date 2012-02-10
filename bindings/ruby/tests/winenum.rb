# winenum.rb
#  enumerate any WMI class in any namespace
#
# Usage
#   winenum [--debug] [--namespace <namespace>] <classname> [ <property> ... ]
#
# Example
# 1. Enumerate namespaces
#   ruby winenum.rb -r -n root __NAMESPACE
#

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
  faults = 0

#  Openwsman::debug = -1  
#  Openwsman.debug = -1  
#  options.max_envelope_size = 1024 * 1024 * 1024
#  puts "max_envelope_size #{options.max_envelope_size}"
  options.set_dump_request if parms[:debug]

  uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/#{namespace}/#{classname}"
  result = client.enumerate( options, nil, uri )
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
    items = body.PullResponse.Items.send classname
    next unless items
    results += 1
    puts items.to_xml if parms[:debug]
    node = items.send classname
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
    puts
  end

  client.release( options, uri, context ) if context
  puts "#{results} results, #{faults} faults"
end

def usage msg=nil
  STDERR.puts msg if msg
  STDERR.puts "Usage:"
  STDERR.puts "winenum [-n <namespace>] [-d] <class>"
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
           [ "--namespace", "-n", GetoptLong::REQUIRED_ARGUMENT ],
           [ "--debug", "-d", GetoptLong::NO_ARGUMENT ]
         )
  opts.each do |opt,arg|
    case opt
    when "--namespace" then parms[:namespace] = arg
    when "--debug" then parms[:debug] = true
    end
  end
rescue GetoptLong::InvalidOption
  usage "invalid option passed"
end

parms[:classname] = ARGV.shift

enum_properties client, parms, *ARGV
