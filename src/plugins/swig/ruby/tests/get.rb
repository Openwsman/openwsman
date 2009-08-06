# get.rb
#   needs 'openwsman-hal' plugin installed
#   see http://repos.opensuse.org/home:/kwk/ for RPM packages.
#

require 'test/unit'
$:.unshift "../../../../../build/bindings/ruby"
$:.unshift "../../../../../bindings/ruby/tests"
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_get
    client = Client.open
    assert client
    puts "Connecting as #{client.user}:#{client.password}"
    options = Openwsman::ClientOptions.new
    assert options
    Openwsman::debug = -1

    schema = "http://schema.opensuse.org/swig/wsman-schema/1-0"
    uri = schema + "/plugin/swig/get/test"

    result = client.get( options, uri )
    assert result

    body = result.body
    assert body

    puts "TEXT #{body.text}"
    puts "RAW #{body}"

  end
end

