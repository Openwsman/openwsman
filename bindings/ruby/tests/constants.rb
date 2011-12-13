# test defined constants

require 'test/unit'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'

class ConstantTest < Test::Unit::TestCase
  def test_constants
    assert_equal     0, Openwsman::FLAG_NONE
    assert_equal     1, Openwsman::FLAG_ENUMERATION_COUNT_ESTIMATION
    assert_equal     2, Openwsman::FLAG_ENUMERATION_OPTIMIZATION
    assert_equal     4, Openwsman::FLAG_ENUMERATION_ENUM_EPR
    assert_equal     8, Openwsman::FLAG_ENUMERATION_ENUM_OBJ_AND_EPR
    assert_equal    16, Openwsman::FLAG_DUMP_REQUEST
    assert_equal    32, Openwsman::FLAG_INCLUDESUBCLASSPROPERTIES
    assert_equal    64, Openwsman::FLAG_EXCLUDESUBCLASSPROPERTIES
    assert_equal   128, Openwsman::FLAG_POLYMORPHISM_NONE
    assert_equal   256, Openwsman::FLAG_MUND_MAX_ESIZE
    assert_equal   512, Openwsman::FLAG_MUND_LOCALE
    assert_equal  1024, Openwsman::FLAG_MUND_OPTIONSET
    assert_equal  2048, Openwsman::FLAG_MUND_FRAGMENT
    assert_equal  4096, Openwsman::FLAG_CIM_EXTENSIONS
    assert_equal  8192, Openwsman::FLAG_CIM_REFERENCES
    assert_equal 16384, Openwsman::FLAG_CIM_ASSOCIATORS
    assert_equal 32768, Openwsman::FLAG_EVENT_SENDBOOKMARK

    assert_equal 0, Openwsman::WSMAN_DELIVERY_PUSH
    assert_equal 1, Openwsman::WSMAN_DELIVERY_PUSHWITHACK
    assert_equal 2, Openwsman::WSMAN_DELIVERY_EVENTS
    assert_equal 3, Openwsman::WSMAN_DELIVERY_PULL
  end
end

