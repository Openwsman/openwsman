# get_class.rb
# Tests an openwsman 2.2 extension: Intrinsic WBEM operations via 'invoke'
#

$:.unshift "../../../bindings/ruby"
$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'openwsman/openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_get_class
    Openwsman::debug = 1
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
#    options.set_dump_request
    
    method = Openwsman::CIM_ACTION_GET_CLASS
    uri = Openwsman::XML_NS_CIM_INTRINSIC + "/CIM_Service"
#    uri = Openwsman::CIM_ALL_AVAILABLE_CLASSES
    
#    options.properties = { "Prefix" => "Linux" }

    result = client.invoke( options, uri, method )
    assert result
#    print result
    unless result.fault?
      node = result.body[method]
      name = node["name"]
      qualifiers = node["qualifiers"]
      properties = node["properties"]
      puts "Class #{name} with #{qualifiers.size} Qualifiers and #{properties.size} Properties"

    else
      puts "--Fault--"
    end
  end
end

