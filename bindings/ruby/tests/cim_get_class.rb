# cim_get_class.rb
#  invoke/GetClass for CIM_ComputerSystem

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'rbwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
#    Rbwsman::debug = 1
    client = Client.open
    assert client
    options = Rbwsman::ClientOptions.new
    assert options
    options.set_dump_request
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
#    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_HostedService"
#    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_CommMechanismForManager"
#    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ManagedElement"
    result = client.get( options, uri )
    assert result
    puts result

  end
end

