# status.rb
# Test WsmanStatus

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class WsmanTest < Test::Unit::TestCase
  def test_status
    status = Openwsman::Status.new
    assert status
    status.code = 42
    assert status.code == 42
    status.detail = 77
    assert status.detail == 77
    status.msg = "Hello, openwsman"
    assert status.msg == "Hello, openwsman"
  end
end

