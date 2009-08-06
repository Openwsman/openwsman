# enumerate.rb

require 'test/unit'
require 'rexml/document'
$:.unshift "../../../../../build/bindings/ruby"
$:.unshift "../../../../../bindings/ruby/tests"
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    context = nil
    client = Client.open
    assert client
    puts "Connecting as #{client.user}:#{client.password}"
    options = Openwsman::ClientOptions.new
    assert options
    uri = "http://schema.opensuse.org/swig/wsman-schema/1-0"
    
    #
    # Enumerate !
    #
    
    result = client.enumerate( options, nil, uri )
    assert result
#    puts "Enum result #{result}"
    context = result.context

loop do
    assert context
    puts "Context: #{context}"
    
    result = client.pull( options, nil, uri, context )
    break unless result
    puts "Pull result #{result}"

    
    body = result.body
    fault = body.find( Openwsman::XML_NS_SOAP_1_2, "Fault" )
    if fault
	puts "Got fault"
	break
    end

    node = body.find( Openwsman::XML_NS_ENUMERATION, "Items" )
    
    puts "Items #{node}"
    
    break if body.find( Openwsman::XML_NS_ENUMERATION, "EndOfSequence")
    puts "Next"
  
    context = result.context
end

    client.release( options, uri, context ) if context

  end
end

