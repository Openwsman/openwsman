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
  def get_owner handle, namespace
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    options.add_selector( "Handle", handle.to_s )

    method = "GetOwner"
    client.invoke( options, namespace, method )

  end

  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
#    options.flags = Openwsman::CLIENTOPTION_DUMP_REQUEST
#    puts "Flags = #{options.flags}"

#
# see http://msdn2.microsoft.com/en-us/library/aa386179.aspx for a list of CIM classes
#   the Win32 classes are derived from
#
    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/CIM_Process"

    result = client.enumerate( options, nil, uri )
    assert result

    results = 0
    faults = 0
    context = nil
 
    printf("%-15s %-15s %10s %10s  %s\n", "User", "Domain", "PID", "VSZ", "Command");
    puts "-"*70
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
	faults += 1
	break
    end

    node = body.PullResponse.Items.Win32_Process

    caption = node.Caption
    handle = node.Handle
    virtual_size = node.VirtualSize
    proc_id = node.ProcessId
    cmd = node.ExecutablePath || caption
    
    user = ""
    domain = ""
    if handle
      ires = get_owner handle, node.ns
      raise ires.to_s if ires.fault?
      user = ires.body.User
      domain = ires.body.Domain
    end
    vsz =  (virtual_size.to_s.to_i / (1024 * 1024 ) ).to_f rescue 0

    printf("%-15s %-15s %10s %10.0f  %s\n", user, domain, proc_id, vsz, cmd);
  end

    client.release( options, uri, context ) if context
    puts "Context released, #{results} results, #{faults} faults"
  end
end

