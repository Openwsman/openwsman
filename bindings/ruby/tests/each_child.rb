# win32_services.rb
#  enumerate/pull/release for Win32_Service
#
# Coded after wsmancli/examples/win32_service.c
#
#

$:.unshift "../../../bindings/ruby"
$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'openwsman/openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
#    options.set_dump_request
#    puts "Flags = #{options.flags}"

#
# see http://msdn2.microsoft.com/en-us/library/aa386179.aspx for a list of CIM classes
#   the Win32 classes are derived from
#
#    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service"
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
    result = client.enumerate( options, nil, uri )
    assert result

    results = 0
    faults = 0
    context = nil
 
#loop do
    context = result.context
    raise "No context inside result" unless context
#    puts "Context #{context} retrieved"

    result = client.pull( options, nil, uri, context )
    break unless result

    results += 1

    if result.fault?
	puts "Got fault"
	faults += 1
	break
    end

    node = result.body.PullResponse.Items.child

    node.each do |child|
      text = child.text
      acount = child.attr_count
      puts "Child [#{acount}] #{child.name}: #{text}"
      if acount > 0
	child.each_attr{ |attr| puts "\tAttr #{attr.ns}:#{attr.name}=#{attr.value}" }
      end
      #	return false if text.nil?
    end
    name = node.Name
    state = node.State

    puts "#{name} is #{state}"
#end

    client.release( options, uri, context ) if context
    puts "Context released, #{results} results, #{faults} faults"
  end
end

