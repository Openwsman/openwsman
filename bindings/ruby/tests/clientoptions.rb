# test ClientOptions class
require 'test/unit'
require '../.libs/rbwsman'

class ClientOptionsTest < Test::Unit::TestCase
  def test_client_options_constructor
    options = Rbwsman::ClientOptions.new
    assert options
  end
end

