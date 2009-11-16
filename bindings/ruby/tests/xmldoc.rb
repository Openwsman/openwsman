# test XmlDoc class

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class XmlDocTest < Test::Unit::TestCase
  def test_doc_constructor
    doc = Openwsman::XmlDoc.new "test"
    assert doc
  end
  def test_doc_rootname
    doc = Openwsman::XmlDoc.new "test"
    assert doc.root.name == "test"
  end
  def test_doc_dump
    doc = Openwsman::XmlDoc.new "test"
    assert doc
    assert doc.to_s
  end
  def test_doc_child_add
    doc = Openwsman::XmlDoc.new "test"
    assert doc
    assert doc.encode "utf-16"
    root = doc.root
    root.add("namespace", "tag")
    root.add("ns", "foo", "bar")
    root.add(nil, "baz", "faz")
    puts "#{doc}"
  end
  def test_doc_accessors
    doc = Openwsman::create_soap_envelope
    assert doc.root
    assert doc.envelope
    assert doc.header
    assert doc.body
    assert doc.element( "Body" )
  end
end

