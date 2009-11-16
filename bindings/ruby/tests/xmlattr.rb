# test XmlAttr class

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class XmlAttrTest < Test::Unit::TestCase
  def test_attr_constructor
    doc = Openwsman::XmlDoc.new "test"
    assert doc
    root = doc.root
    assert root
    # root shouldn't have any attributes
    assert root.attr_count == 0
    # so lets add one
    attr = root.attr_add( Openwsman::XML_NS_SOAP_1_2, "attr", "value" )
    assert attr
    assert attr.ns == Openwsman::XML_NS_SOAP_1_2
    assert attr.name == "attr"
    assert attr.value == "value"
    # if must be there
    assert root.attr_count == 1
    # it must be findable
    attr = root.attr_find( Openwsman::XML_NS_SOAP_1_2, "attr" )
    assert attr.value == "value"
    # it must be enumerable
    root.each_attr { |attr|
      # puts "Attr #{attr.ns}:#{attr.name}=#{attr.value}"
      assert attr
      assert attr.ns
      assert attr.name
      assert attr.value
    }
	
  end
end
