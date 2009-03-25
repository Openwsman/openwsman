# test loading of extension
require 'test/unit'

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

class LoadTest < Test::Unit::TestCase
  def test_loading
    require 'openwsman'
    assert true
  end
end

