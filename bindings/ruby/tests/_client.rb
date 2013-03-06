#
# read client.yml and provide access
#

require 'yaml'

require File.join(File.dirname(__FILE__),'auth-callback')

def show_fault result
  fault = Openwsman::Fault.new result
  STDERR.puts "Fault code #{fault.code}, subcode #{fault.subcode}"
  STDERR.puts "\treason #{fault.reason}"
  STDERR.puts "\tdetail #{fault.detail}"
end

def show_error client
  STDERR.puts "Client failed"
  STDERR.puts "\tResult code #{client.response_code}, Fault: #{client.fault_string.inspect}"
end

def fault? client, result
  if result
    if result.fault?
      show_fault result
      true
    end
  else
    show_error client
    true
  end
end

class Client
  @wsmcs = YAML.load( File.open( File.join(File.dirname(__FILE__),"clients.yml") ) )
  def Client.open( name=nil )
    name = ENV["WSMANCLIENT"] if name.nil?
    name = "localhost" unless name
    client = @wsmcs[name]
    if client.nil?
      STDERR.puts "Client.open unknown name '#{name}'"
      raise "Check WSMANCLIENT environment variable against clients.yml"
    end
    wsmc = Openwsman::Client.new( client["host"], client["port"], client["path"], client["scheme"], client["username"], client["password"] )
    wsmc.transport.timeout = 120
    #
    # Allow to enforce auth scheme
    # e.g. required for Windows which offers BASIC|GSS and Openwsman chooses the strongest (GSS) but non-working
    #
    case client["auth"]
    when nil
      wsmc.transport.auth_method = nil
    when /none/i
      wsmc.transport.auth_method = Openwsman::NO_AUTH_STR
    when /basic/i
      wsmc.transport.auth_method = Openwsman::BASIC_AUTH_STR
    when /digest/i
      wsmc.transport.auth_method = Openwsman::DIGEST_AUTH_STR
    when /pass/i
      wsmc.transport.auth_method = Openwsman::PASS_AUTH_STR
    when /ntlm/i
      wsmc.transport.auth_method = Openwsman::NTLM_AUTH_STR
    when /gss/i
      wsmc.transport.auth_method = Openwsman::GSSNEGOTIATE_AUTH_STR
    else
      raise "Unknown auth_scheme #{@auth_scheme.inspect}"
    end
    
    if client["scheme"] == "https"
      wsmc.transport.verify_peer = 0
      wsmc.transport.verify_host = 0
    end
    
    wsmc
  end
end
