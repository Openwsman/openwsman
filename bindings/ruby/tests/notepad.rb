# invoke.rb

require 'test/unit'
require 'rexml/document'
require '../src/rwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = WsMan::ClientOption.new
    assert options
    options.flags = WsMan::CLIENTOPTION_DUMP_REQUEST

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

