# test WsXmlNode class
require 'test/unit'
require './rbwsman'

class WsXmlNodeTest < Test::Unit::TestCase
  # Nodes are not constructed, but added to other nodes
  def test_node_constructor
    doc = Rbwsman::XmlDoc.new "node"
    assert doc
    root = doc.root
    root.add( Rbwsman::XML_NS_SOAP_1_2, "one" )
    assert root[0].name = "one"
    root.add( Rbwsman::XML_NS_SOAP_1_2, "two", "2" )
    child = root.add( Rbwsman::XML_NS_SOAP_1_2, "three", "drei" )
    assert child.ns == Rbwsman::XML_NS_SOAP_1_2
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
    doc = Rbwsman::create_soap_envelope
    assert doc
    header = doc.element("Header")
    assert header
    assert header.name == "Header"
    assert header.dump
    body = doc.element("Body")
    assert body
    assert body.name == "Body"
    assert body.dump
    envelope = doc.envelope
    assert envelope
    assert envelope.name == "Envelope"
    assert envelope.ns == Rbwsman::XML_NS_SOAP_1_2
  end
end
