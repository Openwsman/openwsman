# enumerate.rb

require 'test/unit'
require 'rexml/document'
require './rbwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    puts "Connecting as #{client.user}:#{client.password}"
    options = Rbwsman::ClientOptions.new
    assert options
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
    result = client.enumerate( options, nil, uri )
    assert result

    doc = REXML::Document.new result.to_s
    doc.write( $stdout, 0 )

    context = result.context
    assert context
    puts "Context: #{context}"
  end
end

