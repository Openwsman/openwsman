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
    context = "924fcbcc-2410-1410-8005-050505050505"
    result = client.pull( options, nil, uri, context )
    assert result

    doc = REXML::Document.new result.to_s
    assert doc
#    doc.write( $stdout, 0 )
  end
end

