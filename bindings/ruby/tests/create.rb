# create.rb

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
    result = client.create( "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_TimeZoneSettingData", options, WsMan::XmlDoc.new )
    assert result

    doc = REXML::Document.new result.rawxml
    assert doc
#    doc.write( $stdout, 0 )
  end
end

