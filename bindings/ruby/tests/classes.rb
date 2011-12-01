# test classes of extension
require 'test/unit'

class LoadTest < Test::Unit::TestCase
  def test_loading
    File.join(File.dirname(__FILE__),'_loadpath')
    require 'openwsman'
    Openwsman.constants.sort.each do |c|
      next if c == c.upcase
      puts c
    end
  end
end

