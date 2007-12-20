# test WsXmlNode class
require 'test/unit'
require '../.libs/rbwsman'

class WsXmlNodeTest < Test::Unit::TestCase
  def test_node_constructor
    doc = Rbwsman::WsXmlDoc.new
    assert doc
    root = doc.root
    assert root
    header = doc.header
    assert header
    body = doc.body
    assert body
  end
  def test_node_accessor
    doc = Rbwsman::WsXmlDoc.new
    assert doc
    header = doc.element("Header")
    assert header
    assert header.name == "Header"
    assert header.dump
    puts "Header #{header}"
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
