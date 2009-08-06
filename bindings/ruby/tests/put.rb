# put.rb

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
    uri = "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_TimeZoneSettingData"
    data = Openwsman::XmlDoc.new("data")
    data_s = data.to_s
    result = client.put( options, uri, data_s, data_s.size, client.encoding )
    assert result

    doc = REXML::Document.new result.to_s
    assert doc
#    doc.write( $stdout, 0 )
  end
end

