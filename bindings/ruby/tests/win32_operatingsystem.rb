# win32_services.rb
#  enumerate/pull/release for Win32_Service
#
# Coded after wsmancli/examples/win32_service.c
#
#

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
#    options.max_envelope_size =  4294967295
#    options.set_dump_request
#    Openwsman.debug = -1
#    puts "Flags = #{options.flags}"

#
# see http://msdn2.microsoft.com/en-us/library/aa386179.aspx for a list of CIM classes
#   the Win32 classes are derived from
#
    klass = "Win32_OperatingSystem"
    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/" + klass

    result = client.enumerate( options, nil, uri )
    assert result

    results = 0
    faults = 0
    context = nil
 
loop do
    context = result.context
    break unless context
#    puts "Context #{context} retrieved"

    result = client.pull( options, nil, uri, context )
    break unless result

    results += 1
    body = result.body
    if result.fault?
	puts "Got fault"
        puts result.to_xml
	faults += 1
	break
    end
#    node = body.child( 0, Openwsman::NS_ENUMERATION, "PullResponse" );
#    node = node.child( 0, Openwsman::NS_ENUMERATION, "Items" );
#    puts node
#    node = node.child( 0, uri, "Win32_Service" );

#    name = node.child( 0, uri, "Name" ).text;
#    state = node.child( 0, uri, "State" ).text;

    node = body.PullResponse.Items.send klass.to_sym
    
    name = node.Name
    puts name
    
#    options.add_selector( "Name", name.to_s )

#    method = "Shutdown"
#    method = "StopService"
#    Openwsman::debug = -1
#    result = client.invoke( options, uri, method )
#    puts result

end

    client.release( options, uri, context ) if context
    puts "Context released, #{results} results, #{faults} faults"
  end
end

