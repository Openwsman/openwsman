# enumerate.rb

require 'test/unit'
require 'rexml/document'
require '../../../../../build/bindings/ruby/rbwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    puts "Connecting as #{client.user}:#{client.password}"
    options = Rbwsman::ClientOptions.new
    assert options
    uri = "http://schema.opensuse.org/swig/wsman-schema/1-0"
    result = client.enumerate( options, nil, uri )
    assert result

    doc = REXML::Document.new result.to_s
    doc.write( $stdout, 0 )

    context = result.context
    assert context
    puts "Context: #{context}"
  end
end

