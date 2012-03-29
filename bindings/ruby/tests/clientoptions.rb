# test ClientOptions class

require 'test/unit'
require File.expand_path(File.join(File.dirname(__FILE__),'_loadpath'))
require 'openwsman'

class ClientOptionsTest < Test::Unit::TestCase
  def test_client_options_constructor
    options = Openwsman::ClientOptions.new
    assert options
    assert_equal Openwsman::FLAG_NONE, options.flags
  end
  def test_client_options_flags
    options = Openwsman::ClientOptions.new
   
    assert_equal Openwsman::FLAG_NONE, options.flags

    # set flag
    options.flags = Openwsman::FLAG_DUMP_REQUEST
    assert_equal Openwsman::FLAG_DUMP_REQUEST, options.flags
    
    # clear flag
    options.clear_flags Openwsman::FLAG_DUMP_REQUEST
    assert_equal Openwsman::FLAG_NONE, options.flags
    
    # reset flag
    options.flags = 0xff
    options.reset_flags
    assert_equal Openwsman::FLAG_NONE, options.flags
  end
end
