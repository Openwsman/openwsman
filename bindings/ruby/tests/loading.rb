# test loading of extension
require 'test/unit'

class LoadTest < Test::Unit::TestCase
  def test_loading
    require '../.libs/rbwsman'
    assert true
  end
end

