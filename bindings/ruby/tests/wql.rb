# wql.rb
#  WS-Man Query Language (for WINRM)
#
# http://msdn.microsoft.com/en-us/library/aa394606(VS.85).aspx

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require File.join(File.dirname(__FILE__), '_client')

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
    filter.wql "select * from meta_class"
  
    result = client.enumerate( options, filter, uri )
    assert result
    if result.fault?
      puts "Enumerate returned fault"
      exit 1
    end

#    puts "Result #{result.to_xml}"
    results = 0
    faults = 0
 
    loop do
      result.Items.each do |node|
        results += 1
        puts node.name
      end
      context = result.context
      break unless context
      result = client.pull( options, nil, uri, context )
      break unless result
    end
    puts "#{results} results, #{faults} faults"
  end
end
