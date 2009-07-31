# nsconsts.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class NsConstsTest < Test::Unit::TestCase
  def test_consts
    assert Openwsman::NS_WSMAN == "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
    assert Openwsman::NS_SCHEMA_INSTANCE
  end
end

