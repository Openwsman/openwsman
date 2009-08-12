# enumerate.rb

$:.unshift "../../../bindings/ruby"
$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'openwsman/openwsman'
require 'auth-callback'

class WsmanTest < Test::Unit::TestCase
  def test_client
#    wsmc = Openwsman::Client.new( client["scheme"], client["host"], client["port"], client["path"], client["username"], client["password"] )
    client = Openwsman::Client.new( "http://wsman:secret@localhost:5985/wsman" )
    client.transport.timeout = 5
    assert client
    puts "Connected as #{client.user}:#{client.password}"
    client.transport.auth_method = Openwsman::BASIC_AUTH_STR

    options = Openwsman::ClientOptions.new
    assert options
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_Process"
    result = client.enumerate( options, nil, uri )
    assert result
    
    first = true
    context = nil

    loop do
      context = result.context
      puts "Result #{result.string}" unless context
      assert context
#      puts "Context: #{context}"

      result = client.pull( options, nil, uri, context )
      break unless result
      
      node = result.body.PullResponse.Items.child
      if first
	node.each_child do |child|
	  text = child.text
	  acount = child.attr_count
	  puts "Child [#{acount}] #{child.name}: #{text}"
	  if acount > 0
	    child.each_attr{ |attr| puts "\tAttr #{attr.ns}:#{attr.name}=#{attr.value}" }
	  end
	  #	return false if text.nil?
	end
	first = false
      end
      
      name = node.Name
      state = node.ExecutionState

      printf "%20s\t%s\n", name, state
      
      break if result.end_of_sequence?
    end
    client.release( options, uri, context ) if context
     
  end
end
