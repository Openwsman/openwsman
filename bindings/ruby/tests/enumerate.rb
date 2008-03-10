# enumerate.rb

require 'test/unit'
require 'rexml/document'
require '../src/rwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    puts "Connecting as #{client.username}:#{client.password}"
    options = WsMan::ClientOption.new
    assert options
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
    result = client.enumerate( uri, options )
    assert result

    doc = REXML::Document.new result.rawxml
    doc.write( $stdout, 0 )

    context = result.context
    assert context
    puts "Context: #{context}"
  end
end

