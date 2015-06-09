# test locale setting

require 'test/unit'
require File.expand_path(File.join(File.dirname(__FILE__),'_loadpath'))
require 'openwsman'
require 'auth-callback'
require '_client'

class LocaleTest < Test::Unit::TestCase
  def test_locale
    Openwsman::debug = 1
    
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
    options.set_dump_request
    
    options.locale = "de_DE"
    assert_equal "de_DE", options.locale
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"

    doc = client.enumerate( options, nil, uri )
    unless doc
      puts "identify failed with #{client.last_error}:#{client.fault_string}"
      raise
    end
    assert doc
    puts "Error!" if doc.fault?
    root = doc.root
    assert root
    
  end
end

