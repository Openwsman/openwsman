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

    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
#    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service"

    service = "Themes"
    options.add_selector( "Name", service )

    method = "StartService"
#    method = "StopService"
    result = client.invoke( options, uri, method )
    unless fault? client, result

      bodychild = result.body.child
    end
  end
end

