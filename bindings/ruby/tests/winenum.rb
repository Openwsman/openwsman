# winenum.rb
#  enumerate/pull/release for Win32_SoftwareElement
#
#

require 'rexml/document'
File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'


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

def enum_properties client, opt, classname, *properties
  options = Openwsman::ClientOptions.new
  uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/#{classname}"
  result = client.enumerate( options, nil, uri )
#  puts result if opt.include? :raw
  puts classname
  unless properties.empty?
    printf "%-#{WIDTH}s" * properties.size, *properties
    puts
    puts "-" * WIDTH * properties.size
  end
  results = 0
  faults = 0
  context = nil
 
  loop do
    context = result.context
    break unless context

    result = client.pull( options, nil, uri, context )
    break unless result

    if result.fault?
        fault = Openwsman::Fault.new result
	puts "Fault code #{fault.code}, subcode #{fault.subcode}"
	puts "\treason #{fault.reason}"
	puts "\tdetail #{fault.detail}"
	faults += 1
	break
    end

    body = result.body
    items = body.PullResponse.Items.send classname
    next unless items
    results += 1
    puts items if opt.include? :raw
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

  end

  client.release( options, uri, context ) if context
  puts "#{results} results, #{faults} faults"
end

client = Client.open

classname = ""
opts = []
loop do
  classname = ARGV.shift
  break unless classname
  break unless classname[0,1] == "-"
  opts << classname[2..-1].to_sym
end

enum_properties client, opts, classname, *ARGV

