#
# read client.yml and provide access
#

require 'yaml'

module Rbwsman
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
	return [ username, password ] if auth_type == Rbwsman::BASIC_AUTH
	return nil     # abort, if non-basic auth
      end
  end
end


class Client
  @@clients = YAML.load( File.open( "clients.yml" ) )
  def Client.open( name=nil )
    name = ENV["WSMANCLIENT"] if name.nil?
    if name.nil?
      STDERR.puts "Client.open without name (set WSMANCLIENT environment to fix this). Defaulting to 'localhost'"
      name = "localhost"
    end
    client = @@clients[name]
    if client.nil?
      STDERR.puts "Client.open unknown name '#{name}'"
      raise "Check WSMANCLIENT environment variable against clients.yml"
    end
    wsmc = Rbwsman::Client.new( client["host"], client["port"], client["path"], client["scheme"], client["username"], client["password"] )
    wsmc.transport.timeout = 5
    wsmc
  end
end
