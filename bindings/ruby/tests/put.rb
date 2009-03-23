# put.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'rbwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = WsMan::ClientOption.new
    assert options
    result = client.put( "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_TimeZoneSettingData", options, WsMan::XmlDoc.new )
    assert result

    doc = REXML::Document.new result.rawxml
    assert doc
#    doc.write( $stdout, 0 )
  end
end

