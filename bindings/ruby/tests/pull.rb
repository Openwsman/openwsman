# enumerate.rb

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
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
    context = "924fcbcc-2410-1410-8005-050505050505"
    result = client.pull( uri, context, options )
    assert result

    doc = REXML::Document.new result.rawxml
    assert doc
#    doc.write( $stdout, 0 )
  end
end

