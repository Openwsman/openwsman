# test find/next operations on XmlNode class

require 'test/unit'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'

class XmlFindTest < Test::Unit::TestCase
  def setup
    @doc = Openwsman::create_doc_from_file("EnumKey.xml")
  end
  def teardown
  end
  def test_doc
    assert @doc
  end
  def test_find
    snames = @doc.sNames
    assert snames
  end
  def test_next
    node = @doc.sNames
    assert node
    next_node = node.next
    assert next_node
  end
  def test_count
    node = @doc.sNames
    assert node
    count = 0
    while node do
     node = node.next
     count += 1
    end
    assert_equal 26, count
  end
end

