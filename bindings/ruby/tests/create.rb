# create.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_create
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
    
    options.set_dump_request
    
    # class name and namespace uri
    name = "OMC_TimeZoneSettingData"
    uri = "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/" + name
    
    # instance values
    instance = { "TimeZone" => "Europe/Berlin" }

    data = Openwsman::XmlDoc.new(name, uri)
    root = data.root
    instance.each do |key,value|
      root.add uri, key, value
    end

    s = data.to_s
    result = client.create( options, uri, s, s.size, "utf-8" )
    puts result

  end
end

