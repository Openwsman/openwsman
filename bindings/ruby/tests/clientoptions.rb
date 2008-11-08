# test ClientOptions class
require 'test/unit'
require './rbwsman'

class ClientOptionsTest < Test::Unit::TestCase
  def test_client_options_constructor
    options = Rbwsman::ClientOptions.new
    assert options
  end
end

