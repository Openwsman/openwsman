# test client class
require 'test/unit'
require '../.libs/rbwsman'

class ClientTest < Test::Unit::TestCase
  def test_client_constructor_uri_simple
    puts "test_uri"
    client = Rbwsman::Client.new( "http://localhost" )
    assert client.scheme == "http"
    assert client.host == "localhost"
  end
  def test_client_constructor_uri
    client = Rbwsman::Client.new( "https://wsman:secret@localhost:8889/wsman" )
    assert client
    assert client.scheme == "https"
    assert client.user == "wsman"
    assert client.password == "secret"
    assert client.host == "localhost"
    assert client.port == 8889
    assert client.path == "/wsman"
  end
  def test_client_constructor_full
    client = Rbwsman::Client.new( "localhost", 8889, "/wsman", "http", "wsman", "secret" )
    assert client
    assert client.scheme == "http"
    assert client.user == "wsman"
    assert client.password == "secret"
    assert client.host == "localhost"
    assert client.port == 8889
    assert client.path == "/wsman"
  end
end

