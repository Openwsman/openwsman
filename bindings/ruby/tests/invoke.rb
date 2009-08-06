# invoke.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options

    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
#    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service"

    service = "Themes"
    options.add_selector( "Name", service )

    method = "StartService"
#    method = "StopService"
    result = client.invoke( options, uri, method )
    assert result

    bodychild = result.body.child
    if result.fault?
      fault = result.fault
      puts "Error!"
#      puts "Code #{fault.code}"
#      puts "Subcode #{fault.subcode}"
#      puts "Reason #{fault.reason}"
#      puts "Detail #{fault.detail}" 
    end
    puts "Result code #{client.response_code}, Fault: #{client.fault_string}"
  end
end

