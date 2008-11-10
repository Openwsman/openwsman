# status.rb
# Test WsmanStatus

require 'test/unit'
require './rbwsman'

class WsmanTest < Test::Unit::TestCase
  def test_status
    status = Rbwsman::Status.new
    assert status
    status.code = 42
    assert status.code == 42
    status.detail = 77
    assert status.detail == 77
    status.msg = "Hello, openwsman"
    assert status.msg == "Hello, openwsman"
  end
end

