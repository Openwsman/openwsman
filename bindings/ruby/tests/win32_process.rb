# win32_services.rb
#  enumerate/pull/release for Win32_Service
#
# Coded after wsmancli/examples/win32_service.c
#
#

require 'test/unit'
require 'rexml/document'
require '../src/rwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def get_owner h
    client = Client.open
    assert client
    options = WsMan::ClientOption.new
    options.selectors = { :Handle => h.to_s }
    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/CIM_Process"

    method = "GetOwner"
    result = client.invoke( uri, method, options )

  end

  def test_client
    client = Client.open
    assert client
    options = WsMan::ClientOption.new
    assert options
#    options.flags = WsMan::CLIENTOPTION_DUMP_REQUEST
#    puts "Flags = #{options.flags}"

#
# see http://msdn2.microsoft.com/en-us/library/aa386179.aspx for a list of CIM classes
#   the Win32 classes are derived from
#
    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/CIM_Process"

    result = client.enumerate( uri, options )
    assert result

    results = 0
    faults = 0
    context = nil
 
    printf("%-20s %-10s %-10s  %s\n", "User", "PID", "VSZ", "Command");
    puts "-------------------------------------------------------------"
loop do
    context = result.context
    break unless context
#    puts "Context #{context} retrieved"

    result = client.pull( uri, context, options )
    break unless result

    results += 1
    body = result.body
    fault = body.child( 0, WsMan::NS_SOAP, "Fault" )
    if fault
	puts "Got fault"
	faults += 1
	break
    end
#    node = body.child( 0, WsMan::NS_ENUMERATION, "PullResponse" );
#    node = node.child( 0, WsMan::NS_ENUMERATION, "Items" );
#    node = node.child( 0, uri, "Win32_Service" );

#    name = node.child( 0, uri, "Name" ).text;
#    state = node.child( 0, uri, "State" ).text;

    node = body.PullResponse.Items.CIM_Process
    caption = node.Caption
    handle = node.Handle
    virtual_size = node.VirtualSize
    proc_id = node.ProcessId
    cmd = node.ExecutablePath

    ires = get_owner handle if handle
    b = ires.body
    output = b.GetOwner_OUTPUT
    user = ""
    output.each_child { |child|
	text = child.text
	if child.name == "User"
		user = text
	end
    }
    vsz =  (virtual_size.to_s.to_i / (1024 * 1024 ) ).to_f

    printf("%-20s %-10s %-5.0f  %s\n", user, proc_id, vsz, cmd);
end

    client.release( uri, context, options ) if context
    puts "Context released, #{results} results, #{faults} faults"
  end
end

