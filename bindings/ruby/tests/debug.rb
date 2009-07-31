# client.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class WsmanTest < Test::Unit::TestCase
  def test_debug
    Openwsman::debug = 5
    assert Openwsman::debug == 5
  end
end

