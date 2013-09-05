# transport.rb

require 'test/unit'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_transport
    client = Client.open
    assert client
    transport = client.transport

    assert transport.auth_method?( Openwsman::BASIC_AUTH )

    assert Openwsman::Transport.auth_name( Openwsman::NO_AUTH ) == Openwsman::NO_AUTH_STR
    assert Openwsman::Transport.auth_name( Openwsman::BASIC_AUTH ) == Openwsman::BASIC_AUTH_STR
    assert Openwsman::Transport.auth_name( Openwsman::DIGEST_AUTH ) == Openwsman::DIGEST_AUTH_STR
    assert Openwsman::Transport.auth_name( Openwsman::PASS_AUTH ) == Openwsman::PASS_AUTH_STR
    assert Openwsman::Transport.auth_name( Openwsman::NTLM_AUTH ) == Openwsman::NTLM_AUTH_STR
    assert Openwsman::Transport.auth_name( Openwsman::GSSNEGOTIATE_AUTH ) == Openwsman::GSSNEGOTIATE_AUTH_STR
    
    transport.agent = "agent"
    assert transport.agent == "agent"

    transport.auth_method = Openwsman::DIGEST_AUTH_STR
    assert transport.auth_method == Openwsman::Transport.auth_name( Openwsman::DIGEST_AUTH )
    assert transport.auth_value == Openwsman::DIGEST_AUTH

    assert Openwsman::Transport.error_string(0)

    transport.timeout = 42
    assert transport.timeout == 42

    transport.verify_peer = false
    assert !transport.verify_peer?
    
    transport.verify_peer = nil
    assert !transport.verify_peer?
    
    transport.verify_peer = 0
    assert !transport.verify_peer?
    
    transport.verify_peer = true
    assert transport.verify_peer?
    transport.verify_peer = "foo"
    assert transport.verify_peer?

    transport.verify_host = true
    assert transport.verify_host?

    transport.proxy = "proxy"
    assert transport.proxy == "proxy"

    transport.proxyauth = "proxy_auth"
    assert transport.proxyauth == "proxy_auth"

    transport.cainfo = "cainfo"
    assert transport.cainfo == "cainfo"

    transport.capath = "capath"
    assert transport.capath == "capath"

    transport.caoid = "caoid"
    assert transport.caoid == "caoid"

    transport.cert = "cert"
    assert transport.cert == "cert"

    transport.key = "key"
    assert transport.key == "key"

  end
end

