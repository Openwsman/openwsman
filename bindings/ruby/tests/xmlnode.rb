# test WsXmlNode class
require 'test/unit'
require './rbwsman'

class WsXmlNodeTest < Test::Unit::TestCase
  # Nodes are not constructed, but added to other nodes
  def test_node_constructor
    doc = Rbwsman::XmlDoc.new
    assert doc
    body = doc.body
    assert body
    body.child_add( Rbwsman::XML_NS_SOAP_1_2, "one" )
    body.child_add( Rbwsman::XML_NS_SOAP_1_2, "two", "2" )
    child = body.child_add( Rbwsman::XML_NS_SOAP_1_2, "three", "drei" )
    assert child.ns == Rbwsman::XML_NS_SOAP_1_2
    assert child.name == "three"
    assert child.text == "drei"
    assert body.child_count == 3
    child.text = "troi"
    assert child.text == "troi"
    i = 0
    body.each_child { |c| i += 1 }
    assert i == 3
  end
  def test_node_accessor
    doc = Rbwsman::XmlDoc.new
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
