# test WsXmlDoc class
require 'test/unit'
require './rbwsman'

class WsXmlDocTest < Test::Unit::TestCase
  def test_doc_constructor
    doc = Rbwsman::XmlDoc.new
    assert doc
  end
  def test_doc_dump
    doc = Rbwsman::XmlDoc.new
    assert doc
    assert doc.dump
  end
  def test_doc_accessors
    doc = Rbwsman::XmlDoc.new
    assert doc.root
    assert doc.envelope
    assert doc.header
    assert doc.body
    assert doc.element( "Body" )
  end
end

