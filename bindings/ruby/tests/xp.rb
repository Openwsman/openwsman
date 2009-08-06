# identify.rb

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'
require '_client'

require 'auth-callback'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Openwsman::Client.new( "http://wsman:secret@192.168.0.3:80/wsman" )
    assert client
    puts "Connected as #{client.user}:#{client.password}"

    options = Openwsman::ClientOptions.new
#    Openwsman::debug = -1
#    options.cim_ns = "http://schema.opensuse.org/YaST/wsman-schema/10-3"
  
    result = client.identify( options )
    assert result
    puts "#{result.product_vendor} #{result.product_version} supporting protocol #{result.protocol_version}"

    puts "RAW #{result}"
#    puts "HEADER #{result.header}"
#    puts "BODY #{result.body}"
#    vendor = result.element "ProductVendor"
#    puts "ProductVendor #{vendor}" if vendor
  end
end

