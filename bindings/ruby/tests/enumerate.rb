# enumerate.rb

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
    result = client.enumerate( options, nil, uri )
    assert result

    doc = REXML::Document.new result.to_xml
    doc.write( $stdout, 0 )

    context = result.context
    assert context

    client.release( options, uri, context)
  end
end

