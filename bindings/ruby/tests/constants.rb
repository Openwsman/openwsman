# test defined constants

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class ConstantTest < Test::Unit::TestCase
  def test_constants
    assert Openwsman::FLAG_NONE == 0
    assert Openwsman::FLAG_ENUMERATION_COUNT_ESTIMATION == 1
    assert Openwsman::FLAG_ENUMERATION_OPTIMIZATION == 2
    assert Openwsman::FLAG_ENUMERATION_ENUM_EPR == 4
    assert Openwsman::FLAG_ENUMERATION_ENUM_OBJ_AND_EPR == 8
    assert Openwsman::FLAG_DUMP_REQUEST == 16
    assert Openwsman::FLAG_INCLUDESUBCLASSPROPERTIES == 32
    assert Openwsman::FLAG_EXCLUDESUBCLASSPROPERTIES == 64
    assert Openwsman::FLAG_POLYMORPHISM_NONE == 128
    assert Openwsman::FLAG_MUND_MAX_ESIZE == 256
    assert Openwsman::FLAG_MUND_LOCALE == 512
    assert Openwsman::FLAG_MUND_OPTIONSET == 1024
    assert Openwsman::FLAG_MUND_FRAGMENT == 2048
    assert Openwsman::FLAG_CIM_EXTENSIONS == 4096
    assert Openwsman::FLAG_CIM_REFERENCES == 8192
    assert Openwsman::FLAG_CIM_ASSOCIATORS == 16384
    assert Openwsman::FLAG_EVENT_SENDBOOKMARK == 32768
    assert Openwsman::WSMAN_DELIVERY_PUSH == 0
    assert Openwsman::WSMAN_DELIVERY_PUSHWITHACK == 1
    assert Openwsman::WSMAN_DELIVERY_EVENTS == 2
    assert Openwsman::WSMAN_DELIVERY_PULL == 3
  end
end

