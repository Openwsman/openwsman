# test client class
require 'test/unit'
require '../.libs/rbwsman'

class LoadTest < Test::Unit::TestCase
  def test_client_constructor_uri
    client = Rbwsman::Client.new( "http://wsman:secret@localhost:8889/wsman" )
    assert client
    assert client.scheme == "http"
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

