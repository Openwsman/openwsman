# test WsXmlDoc class
require 'test/unit'
require '../.libs/rbwsman'

class WsXmlDocTest < Test::Unit::TestCase
  def test_doc_constructor
    doc = Rbwsman::WsXmlDoc.new
    assert doc
  end
  def test_doc_dump
    doc = Rbwsman::WsXmlDoc.new
    assert doc
    assert doc.dump
    puts doc.dump
  end
end

