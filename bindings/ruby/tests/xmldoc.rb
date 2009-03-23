# test WsXmlDoc class

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rbwsman'

class WsXmlDocTest < Test::Unit::TestCase
  def test_doc_constructor
    doc = Rbwsman::XmlDoc.new "test"
    assert doc
  end
  def test_doc_rootname
    doc = Rbwsman::XmlDoc.new "test"
    assert doc.root.name == "test"
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
    root.add("namespace", "tag")
    root.add("ns", "foo", "bar")
    root.add(nil, "baz", "faz")
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

