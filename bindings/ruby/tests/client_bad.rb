# client.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
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

