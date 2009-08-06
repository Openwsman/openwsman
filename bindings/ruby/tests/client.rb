# test client class

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class ClientTest < Test::Unit::TestCase
  def test_client_constructor_uri_simple
    puts "test_uri"
    client = Openwsman::Client.new( "http://localhost" )
    assert client.scheme == "http"
    assert client.host == "localhost"
  end
  def test_client_constructor_uri
    client = Openwsman::Client.new( "https://wsman:secret@localhost:5985/wsman" )
    assert client
    assert client.scheme == "https"
    assert client.user == "wsman"
    assert client.password == "secret"
    assert client.host == "localhost"
    assert client.port == 5985
    assert client.path == "/wsman"
  end
  def test_client_constructor_full
    client = Openwsman::Client.new( "localhost", 5985, "/wsman", "http", "wsman", "secret" )
    assert client
    assert client.scheme == "http"
    assert client.user == "wsman"
    assert client.password == "secret"
    assert client.host == "localhost"
    assert client.port == 5985
    assert client.path == "/wsman"
  end
  def test_client_client
    require '_client'
    assert Client.open
  end
  def test_client_encoding
    client = Openwsman::Client.new( "localhost", 5985, "/wsman", "http", "wsman", "secret" )
    assert client.encoding
    client.encoding = "UTF-16"
    assert_equal client.encoding, "UTF-16" 
  end
end
