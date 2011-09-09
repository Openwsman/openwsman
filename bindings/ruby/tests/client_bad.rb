# client.rb

require 'test/unit'
require '_loadpath'
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    begin
      client = Client.open( "unknown" )
    rescue
    end 
    assert !client
  end
end

