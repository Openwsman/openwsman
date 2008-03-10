# transport.rb

require 'test/unit'
require '../src/rwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_transport
    client = Client.open
    assert client
    transport = client.transport

    begin
      t = WsMan::Transport.new( client )
      assert false	# Transport.new() must raise
    rescue RuntimeError
      assert true
    end

    assert transport.auth_method?( WsMan::Transport::AUTH_BASIC )
    assert WsMan::Transport.auth_name( WsMan::Transport::AUTH_DIGEST ) == "Digest"

    assert transport.agent = "agent"
    assert transport.agent == "agent"

    assert transport.auth_method = "Digest"
    assert transport.auth_method == WsMan::Transport.auth_name( WsMan::Transport::AUTH_DIGEST )
    assert transport.auth_value == WsMan::Transport::AUTH_DIGEST

    assert WsMan::Transport.error_string(0)

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

