# enumerate.rb

require 'test/unit'
require 'rexml/document'
require '../src/rwsman'

class WsmanTest < Test::Unit::TestCase
  def test_client
#    wsmc = WsMan::Client.new( client["scheme"], client["host"], client["port"], client["path"], client["username"], client["password"] )
    client = WsMan::Client.new( "http://pegasus:secret@localhost:8889/wsman" )
    client.transport.timeout = 5
    assert client
    puts "Connected as #{client.username}:#{client.password}"
    options = WsMan::ClientOption.new
    assert options
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_Process"
    result = client.enumerate( uri, options )
    assert result
    
    first = true
    
    loop do
      context = result.context
      assert context
#      puts "Context: #{context}"

      result = client.pull( uri, context, options )
      break unless result
      
      node = result.body.PullResponse.Items.child
      if first
	node.each_child { |child|
	  text = child.text
	  acount = child.attr_count
	  puts "Child [#{acount}] #{child.name}: #{text}"
	  if acount > 0
	    child.each_attr{ |attr| puts "\tAttr #{attr.ns}:#{attr.name}=#{attr.value}" }
	  end
	  #	return false if text.nil?
	}
	first = false
      end
      
      name = node.Name
      state = node.ExecutionState

      printf "%20s\t%s\n", name, state
    end
  end
end

