# invoke.rb

require 'test/unit'
require 'rexml/document'
File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
    options.set_dump_request

    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Process"

    options.add_selector( "CommandLine", "notepad.exe")
    options.add_selector( "CurrentDirectory", "C:\\" )

    method = "Create"
#    method = "StopService"
    result = client.invoke( options, uri, method )
    assert result

    puts "Result code #{client.response_code}, Fault: #{client.fault_string}"
    puts "#{result}"
  end
end

