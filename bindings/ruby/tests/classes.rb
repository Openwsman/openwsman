# test classes of extension
require 'test/unit'

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"
$:.unshift ".."

class LoadTest < Test::Unit::TestCase
  def test_loading
    require 'openwsman/openwsman'
    Openwsman.constants.sort.each do |c|
      next if c == c.upcase
      puts c
    end
  end
end

