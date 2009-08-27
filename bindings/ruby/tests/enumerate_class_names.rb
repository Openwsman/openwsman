# enumerate_class_names.rb
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
  def test_enumerate_class_names
    Openwsman::debug = 1
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    options.set_dump_request
    
#    options.selectors = { Openwsman::CIM_NAMESPACE_SELECTOR => "cimv2/interop" }
    
    method = Openwsman::CIM_ACTION_ENUMERATE_CLASS_NAMES
    uri = Openwsman::XML_NS_CIM_INTRINSIC
    
#    options.properties = { "Prefix" => "Linux" }

    result = client.invoke( options, uri, method )
    assert result
    unless result.fault?
      output = result.body[method]
      classes = []
      output.each do |c|
	classes << c.to_s
      end
      puts "Classes"
      puts classes.inspect
    else
      puts result
    end
  end
end

