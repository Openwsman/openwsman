# wql.rb
#  WS-Man Query Language (for WINRM)
#
# http://msdn.microsoft.com/en-us/library/aa394606(VS.85).aspx

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require File.join(File.dirname(__FILE__), '_client')

def handle_fault client, result
  unless result
    if client.last_error != 0
      STDERR.puts "Client connection to #{client.scheme}://#{client.user}:#{client.password}@#{client.host}:#{client.port}/#{client.path} failed with #{client.last_error}, Fault: #{client.fault_string}"
      exit 1
    end
    if client.response_code != 200
      STDERR.puts "Client requested result #{client.response_code}, Fault: #{client.fault_string}"
      exit 1
    end
    STDERR.puts "Client action failed for unknown reason"
    exit 1
  end
  if result.fault?
    fault = Openwsman::Fault.new result
    STDERR.puts "Fault code #{fault.code}, subcode #{fault.subcode}"
    STDERR.puts "\treason #{fault.reason}"
    STDERR.puts "\tdetail #{fault.detail}"
    exit 1
  end
end

class WsmanTest < Test::Unit::TestCase

  def test_wql
    client = Client.open
    options = Openwsman::ClientOptions.new
    options.flags = Openwsman::FLAG_ENUMERATION_OPTIMIZATION
    options.max_elements = 999
#    options.set_dump_request

    namespace = "root/cimv2"
    classname = "*" # must be '*' for WQL
    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/#{namespace}/#{classname}"
  
    filter = Openwsman::Filter.new
    # see winenum.rb for more examples
    filter.wql "select * from Win32_NetworkAdapterConfiguration where IPEnabled=true"
  
    result = client.enumerate( options, filter, uri )
    handle_fault client, result
    assert result
    if result.fault?
      puts "Enumerate returned fault"
      puts "\t #{client.last_error}:#{client.fault_string}"
      exit 1
    end

#    puts "Result #{result.to_xml}"
    results = 0
    faults = 0
 
    loop do
      result.Items.each do |node|
        results += 1
        puts "#{node.name}: IPEnabled: #{node.IPEnabled}"
      end
      context = result.context
      break unless context
      result = client.pull( options, nil, uri, context )
      break unless result
    end
    puts "#{results} results, #{faults} faults"
  end
end
