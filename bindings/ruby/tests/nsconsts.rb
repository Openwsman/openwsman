# nsconsts.rb

require 'test/unit'
require '../src/rwsman'

class NsConstsTest < Test::Unit::TestCase
  def test_consts
    assert WsMan::NS_WSMAN == "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
    assert WsMan::NS_SCHEMA_INSTANCE
  end
end

