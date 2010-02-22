# subscribe.rb
# generate listing 9 of http://www.ibm.com/developerworks/webservices/library/ws-CIM/index.html

$:.unshift "../../../bindings/ruby"
$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'openwsman/openwsman'
require 'auth-callback'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    Openwsman.debug = -1
    client = Client.open
    assert client
    client.transport.timeout = 5
    puts "Connected as #{client.user}:#{client.password}"

    options = Openwsman::ClientOptions.new
    assert options
    options.set_dump_request # dump XML request
    options.add_selector( Openwsman::CIM_NAMESPACE_SELECTOR, "root/interop" )
    # subscription expires after 120 seconds
    options.sub_expiry = 120
    # Default would be 'Push'
    options.delivery_mode = Openwsman::WSMAN_DELIVERY_PULL
    options.delivery_uri = "http://127.0.0.1:80/eventsink"
    uri = Openwsman::CIM_ALL_AVAILABLE_CLASSES
    filter = Openwsman::Filter.new
    filter.wql("SELECT * FROM CIM_ProcessIndication")
    
    result = client.subscribe( options, filter, uri )
#    assert result
    
  end
end
