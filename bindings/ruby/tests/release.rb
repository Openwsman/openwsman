# release.rb

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
    context = "1e9ba1a2-2412-1412-8002-020202020202"
    result = client.release( options, uri, context )
    assert result

    doc = REXML::Document.new result.to_s
    assert doc
#    doc.write( $stdout, 0 )
  end
end

