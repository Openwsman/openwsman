# put.rb

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
#    Openwsman.debug = -1
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
#    options.set_dump_request
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_TimeZoneSettingData"
    data = Openwsman::XmlDoc.new("data")
    data_s = data.to_xml
    result = client.put( options, uri, data_s, data_s.size, client.encoding )

    doc = REXML::Document.new result.to_xml
    assert doc
#    doc.write( $stdout, 0 )
  end
end

