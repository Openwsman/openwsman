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
    options.flags = Openwsman::CLIENTOPTION_DUMP_REQUEST

    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Process"

    options.properties = { "CommandLine" => "notepad.exe", "CurrentDirectory" => "C:\\" }

    method = "Create"
#    method = "StopService"
    result = client.invoke( uri, method, options )
    assert result

    puts "Result code #{client.response_code}, Fault: #{client.fault_string}"
    puts "#{result.rawxml}"
  end
end

