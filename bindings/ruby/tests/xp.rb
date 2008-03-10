# identify.rb

require 'test/unit'
require '../src/rwsman'
require '_client'

#
# define transport authorization callback, provide username & password interactively
#

module WsMan
  class Transport
      def Transport.auth_request_callback( client, auth_type )
	puts "Transport.auth_request_callback( #{client}, #{auth_type} )"
	puts Transport.auth_name( 1 )
	puts "#{Transport.auth_name( auth_type )} authentication failed for #{client.host}"
	print "Username:"
	STDOUT.flush
	username = STDIN.gets.chomp
	print "Password:"
	STDOUT.flush
	password = STDIN.gets.chomp
	return [ username, password ] if auth_type == Transport::AUTH_BASIC
	return nil     # abort, if non-basic auth
      end
  end
end

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = WsMan::Client.new( "http://wsman:secret@192.168.0.3:80/wsman" )
    assert client
    puts "Connected as #{client.username}:#{client.password}"

    options = WsMan::ClientOption.new
#    WsMan::debug = -1
    options.cim_ns = "http://schema.opensuse.org/YaST/wsman-schema/10-3"
  
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

