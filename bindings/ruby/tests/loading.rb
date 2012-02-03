# test loading of extension
require 'test/unit'


class LoadTest < Test::Unit::TestCase
  def test_loading
    require File.join(File.dirname(__FILE__),'_loadpath')
    require 'openwsman'
    assert true
  end
end

