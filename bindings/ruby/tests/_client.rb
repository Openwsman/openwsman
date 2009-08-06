#
# read client.yml and provide access
#

require 'yaml'

class Client
  @@clients = YAML.load( File.open( File.join(File.dirname(__FILE__),"clients.yml") ) )
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
    wsmc = Openwsman::Client.new( client["host"], client["port"], client["path"], client["scheme"], client["username"], client["password"] )
    wsmc.transport.timeout = 5
    wsmc.transport.auth_method = Openwsman::BASIC_AUTH_STR
    wsmc
  end
end
