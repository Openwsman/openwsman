# test ClientOptions class

require 'test/unit'
require '_loadpath'
require 'openwsman'

class ClientOptionsTest < Test::Unit::TestCase
  def test_client_options_constructor
    options = Openwsman::ClientOptions.new
    assert options
  end
end

