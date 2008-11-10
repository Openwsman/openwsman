# test WsXmlDoc class
require 'test/unit'
require './rbwsman'

class WsXmlDocTest < Test::Unit::TestCase
  def test_doc_constructor
    doc = Rbwsman::XmlDoc.new "test"
    assert doc
  end
  def test_doc_dump
    doc = Rbwsman::XmlDoc.new "test"
    assert doc
    assert doc.dump
  end
  def test_doc_child_add
    doc = Rbwsman::XmlDoc.new "test"
    assert doc
    assert doc.dump
    root = doc.root
    root.child_add("namespace", "tag")
    root.child_add("ns", "foo", "bar")
    root.child_add(nil, "baz", "faz")
    puts "#{doc}"
  end
  def test_doc_accessors
    doc = Rbwsman::create_soap_envelope
    assert doc.root
    assert doc.envelope
    assert doc.header
    assert doc.body
    assert doc.element( "Body" )
  end
end

