# client.rb

require 'test/unit'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'

class WsmanTest < Test::Unit::TestCase
  def test_debug
    Openwsman::debug = 5
    assert Openwsman::debug == 5
  end
end

