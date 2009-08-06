# get.rb

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
#    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
    uri = "http://schema.openwbem.org/wbem/wscim/1/cim-schema/2/OpenWBEM_UnitaryComputerSystem"
#    uri = "http://schema.openwbem.org/wbem/wscim/1/cim-schema/2/OMC_UnitaryComputerSystem"
    options.add_selector( "Name", "heron.suse.de" );
    options.add_selector( "CreationClassName", "OpenWBEM_UnitaryComputerSystem" );
    result = client.get( options, uri )
    assert result

    puts result
  end
end

