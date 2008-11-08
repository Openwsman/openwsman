# create.rb

require 'test/unit'
require 'rexml/document'
require './rbwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Rbwsman::ClientOptions.new
    assert options
    s = Rbwsman::XmlDoc.new.to_s
    result = client.create( options, "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/OMC_TimeZoneSettingData", s, s.size, "utf-8" )
    assert result

    doc = REXML::Document.new result.to_s
    assert doc
#    doc.write( $stdout, 0 )
  end
end

