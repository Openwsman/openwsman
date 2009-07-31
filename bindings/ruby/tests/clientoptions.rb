# test ClientOptions class

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class ClientOptionsTest < Test::Unit::TestCase
  def test_client_options_constructor
    options = Openwsman::ClientOptions.new
    assert options
  end
end

