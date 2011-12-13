# nsconsts.rb

require 'test/unit'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'

class NsConstsTest < Test::Unit::TestCase
  def test_consts
    assert Openwsman::XML_NS_WS_MAN == "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
    assert Openwsman::XML_NS_SCHEMA_INSTANCE
  end
end

