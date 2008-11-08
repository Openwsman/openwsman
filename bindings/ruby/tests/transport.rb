# transport.rb

require 'test/unit'
require './rbwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_transport
    client = Client.open
    assert client
    transport = client.transport

    assert transport.auth_method?( Rbwsman::BASIC_AUTH )
    assert Rbwsman::Transport.auth_name( Rbwsman::DIGEST_AUTH ) == Rbwsman::DIGEST_AUTH_STR

    assert transport.agent = "agent"
    assert transport.agent == "agent"

    assert transport.auth_method = Rbwsman::DIGEST_AUTH_STR
    assert transport.auth_method == Rbwsman::Transport.auth_name( Rbwsman::DIGEST_AUTH )
    assert transport.auth_value == Rbwsman::DIGEST_AUTH

    assert Rbwsman::Transport.error_string(0)

    assert transport.timeout = 42
    assert transport.timeout == 42

    assert transport.verify_peer = 42
    assert transport.verify_peer == 42

    assert transport.verify_host = 42
    assert transport.verify_host == 42

    assert transport.proxy = "proxy"
    assert transport.proxy == "proxy"

    assert transport.proxyauth = "proxy_auth"
    assert transport.proxyauth == "proxy_auth"

    assert transport.cainfo = "cainfo"
    assert transport.cainfo == "cainfo"

    assert transport.capath = "capath"
    assert transport.capath == "capath"

    assert transport.caoid = "caoid"
    assert transport.caoid == "caoid"

    assert transport.cert = "cert"
    assert transport.cert == "cert"

    assert transport.key = "key"
    assert transport.key == "key"

  end
end

