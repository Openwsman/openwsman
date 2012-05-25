# invoke.rb

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
    options.set_dump_request

    uri_prefix = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/"
    klass = "Win32_ProcessStartup"
    uri = uri_prefix + klass
    
    # instance values
    instance = { "Title" => "Notepad" }

    data = Openwsman::XmlDoc.new(klass, uri)
    root = data.root
    instance.each do |key,value|
      root.add uri, key, value
    end

    s = data.to_xml

#    puts "Creating #{klass} with #{s}"
#
#    result = client.create( options, uri, s, s.size, "utf-8" )
#    if fault? client, result
#      exit 1
#    end

    puts "Starting notepad"

    uri = uri_prefix + "Win32_Process"

    options.add_selector( "CommandLine", "notepad.exe")
    options.add_selector( "CurrentDirectory", "C:\\" )
    options.add_selector( "ProcessStartupInformation", s )

    method = "Create"
#    method = "StopService"
    result = client.invoke( options, uri, method )
    fault? client, result

  end
end

