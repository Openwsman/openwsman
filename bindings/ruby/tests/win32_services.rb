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
require 'auth-callback'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
    options.timeout = 30
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
    if result.fault?
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

    node = result.body.find( "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service", "Win32_Service" )
#    puts node.to_xml
    name = node.find( "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service", "Name" )
    caption = node.find( "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service", "Caption" )
    state = node.find( "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service", "State" )

    puts "#{name} (#{caption}) is #{state}"
end

    client.release( options, uri, context ) if context
    puts "Context released, #{results} results, #{faults} faults"
  end
end

