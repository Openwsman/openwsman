# test ClientOptions class

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rbwsman'

class ClientOptionsTest < Test::Unit::TestCase
  def test_client_options_constructor
    options = Rbwsman::ClientOptions.new
    assert options
  end
end

