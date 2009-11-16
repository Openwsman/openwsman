# test XmlNode class

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class XmlNodeTest < Test::Unit::TestCase
  # Nodes are not constructed, but added to other nodes
  def test_node_constructor
    doc = Openwsman::XmlDoc.new "node"
    assert doc
    root = doc.root
    root.add( Openwsman::XML_NS_SOAP_1_2, "one" )
    assert root[0].name = "one"
    root.add( Openwsman::XML_NS_SOAP_1_2, "two", "2" )
    child = root.add( Openwsman::XML_NS_SOAP_1_2, "three", "drei" )
    assert child.ns == Openwsman::XML_NS_SOAP_1_2
    assert child.name == "three"
    assert child.text == "drei"
    assert root.size == 3
    child.text = "troi"
    assert child.text == "troi"
    assert root[2] == child
    i = 0
    root.each { |c| i += 1 }
    assert i == root.size
  end
  def test_node_accessor
    doc = Openwsman::create_soap_envelope
    assert doc
    header = doc.element("Header")
    assert header
    assert header.name == "Header"
    assert header.to_s
    body = doc.element("Body")
    assert body
    assert body.name == "Body"
    assert body.to_s
    envelope = doc.envelope
    assert envelope
    assert envelope.name == "Envelope"
    assert envelope.ns == Openwsman::XML_NS_SOAP_1_2
  end
  def test_node_add
    doc = Openwsman::XmlDoc.new "node"
    root = doc.root
    node = root.add("ns", "name", "value")
    root << node
    root << node
    root << node
    assert root.size == 4
    assert root[1] != node # its a dup
    assert root[0].name == root[1].name
    puts doc
  end
end
