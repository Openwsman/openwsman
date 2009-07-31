# win32_services.rb
#  enumerate/pull/release for Win32_Service
#
# Coded after wsmancli/examples/win32_service.c
#
#

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'openwsman'
require '_client'
require 'auth-callback'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
#    options.set_dump_request

#
# see http://msdn2.microsoft.com/en-us/library/aa386179.aspx for a list of CIM classes
#   the Win32 classes are derived from
#
    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service"

    result = client.enumerate( options, nil, uri )
    assert result

    results = 0
    faults = 0
    context = nil
 
loop do
    context = result.context
    break unless context
#    puts "Context #{context} retrieved"

    filter = nil
    result = client.pull( options, filter, uri, context )
    break unless result

    results += 1
    body = result.body
    fault = body.find( Openwsman::XML_NS_SOAP_1_1, "Fault", 1 )
    if fault
      puts "Got fault"
      faults += 1
      break
    end
      
#    puts body.string
    
#    node = body.child( 0, Openwsman::NS_ENUMERATION, "PullResponse" );
#    node = node.child( 0, Openwsman::NS_ENUMERATION, "Items" );
#    node = node.child( 0, uri, "Win32_Service" );

#    name = node.child( 0, uri, "Name" ).text;
#    state = node.child( 0, uri, "State" ).text;

    node = body.find( "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service", "Win32_Service" )
    name = node.find( "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service", "Name" )
    state = node.find( "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service", "State" )

    puts "#{name} is #{state}"
end

    client.release( uri, context, options ) if context
    puts "Context released, #{results} results, #{faults} faults"
  end
end

