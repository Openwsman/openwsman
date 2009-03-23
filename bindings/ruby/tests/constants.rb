# test defined constants

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rbwsman'

class ConstantTest < Test::Unit::TestCase
  def test_constants
    assert Rbwsman::FLAG_NONE == 0
    assert Rbwsman::FLAG_ENUMERATION_COUNT_ESTIMATION == 1
    assert Rbwsman::FLAG_ENUMERATION_OPTIMIZATION == 2
    assert Rbwsman::FLAG_ENUMERATION_ENUM_EPR == 4
    assert Rbwsman::FLAG_ENUMERATION_ENUM_OBJ_AND_EPR == 8
    assert Rbwsman::FLAG_DUMP_REQUEST == 16
    assert Rbwsman::FLAG_INCLUDESUBCLASSPROPERTIES == 32
    assert Rbwsman::FLAG_EXCLUDESUBCLASSPROPERTIES == 64
    assert Rbwsman::FLAG_POLYMORPHISM_NONE == 128
    assert Rbwsman::FLAG_MUND_MAX_ESIZE == 256
    assert Rbwsman::FLAG_MUND_LOCALE == 512
    assert Rbwsman::FLAG_MUND_OPTIONSET == 1024
    assert Rbwsman::FLAG_MUND_FRAGMENT == 2048
    assert Rbwsman::FLAG_CIM_EXTENSIONS == 4096
    assert Rbwsman::FLAG_CIM_REFERENCES == 8192
    assert Rbwsman::FLAG_CIM_ASSOCIATORS == 16384
    assert Rbwsman::FLAG_EVENT_SENDBOOKMARK == 32768
    assert Rbwsman::WSMAN_DELIVERY_PUSH == 0
    assert Rbwsman::WSMAN_DELIVERY_PUSHWITHACK == 1
    assert Rbwsman::WSMAN_DELIVERY_EVENTS == 2
    assert Rbwsman::WSMAN_DELIVERY_PULL == 3
  end
end

