# nsconsts.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class NsConstsTest < Test::Unit::TestCase
  def test_consts
    assert Openwsman::XML_NS_WS_MAN == "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
    assert Openwsman::XML_NS_SCHEMA_INSTANCE
  end
end

