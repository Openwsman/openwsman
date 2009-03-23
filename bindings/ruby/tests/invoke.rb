# invoke.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'rbwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = WsMan::ClientOption.new
    assert options

    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
#    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service"

    service = "Themes"
    options.selector_add( "Name", service )

    method = "StartService"
#    method = "StopService"
    result = client.invoke( uri, method, options )
    assert result

    bodychild = result.body.child
    puts "Error" if bodychild.name == "Fault"

    puts "Result code #{client.response_code}, Fault: #{client.fault_string}"
  end
end

