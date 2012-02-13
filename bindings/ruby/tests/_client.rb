#
# read client.yml and provide access
#

require 'yaml'

require 'auth-callback'

class Client
  @@clients = YAML.load( File.open( File.join(File.dirname(__FILE__),"clients.yml") ) )
  def Client.open( name=nil )
    name = ENV["WSMANCLIENT"] if name.nil?
    name = "localhost" unless name
    client = @@clients[name]
    if client.nil?
      STDERR.puts "Client.open unknown name '#{name}'"
      raise "Check WSMANCLIENT environment variable against clients.yml"
    end
    wsmc = Openwsman::Client.new( client["host"], client["port"], client["path"], client["scheme"], client["username"], client["password"] )
    wsmc.transport.timeout = 5
    if client["auth"] == "digest"
      wsmc.transport.auth_method = Openwsman::DIGEST_AUTH_STR
    else
      wsmc.transport.auth_method = Openwsman::BASIC_AUTH_STR
    end
    if client["scheme"] == "https"
      wsmc.transport.key = "serverkey.pem"
      wsmc.transport.cert = "servercert.pem"
      wsmc.transport.verify_peer = 0
      wsmc.transport.verify_host = 0
    end
    
    wsmc
  end
end
