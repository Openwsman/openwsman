# winenum.rb
#  enumerate/pull/release for Win32_SoftwareElement
#
#

$:.unshift "../../../bindings/ruby"
$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'rexml/document'
require 'openwsman/openwsman'
require '_client'

WIDTH = 25

def enum_properties client, classname, *properties
  options = Openwsman::ClientOptions.new
  uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/#{classname}"
  result = client.enumerate( options, nil, uri )

  puts classname
  printf "%-#{WIDTH}s" * properties.size, *properties
  puts
  puts "-" * WIDTH * properties.size

  results = 0
  faults = 0
  context = nil
 
  loop do
    context = result.context
    break unless context

    result = client.pull( options, nil, uri, context )
    break unless result

    results += 1
    body = result.body
    if result.fault?
	puts "Got fault"
	faults += 1
	break
    end

    node = body.PullResponse.Items.send classname
    values = []
    properties.each do |p|
      values << node.send(p)
    end
    printf "%-#{WIDTH}s" * values.size, *values
    puts

  end

  client.release( options, uri, context ) if context
#  puts "Context released, #{results} results, #{faults} faults"
  puts
end

client = Client.open
    
classname = ARGV.shift
enum_properties client, classname, *ARGV

