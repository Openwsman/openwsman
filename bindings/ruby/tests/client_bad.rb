# client.rb

require 'test/unit'
require 'rbwsman'
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

