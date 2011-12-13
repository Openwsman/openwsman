# cim_computer_system.rb
#  enumerate/pull/release for CIM_OperatingSystem

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
#    options.set_dump_request
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_OperatingSystem"
#    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_HostedService"
#    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_CommMechanismForManager"
#    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ManagedElement"
    result = client.enumerate( options, nil, uri )
    assert result
#    puts result

    count = 0
    context = result.context
    while (context)
	count += 1
	result = client.pull( options, nil, uri, context )
	puts "Response ##{count}"
#	puts "### #{result}"
	result.body["PullResponse"]["Items"].child.each { |child|
	    puts "\t#{child.name} = #{child.text}"
	}
	context = result.context
    end
    client.release( options, uri, context ) if context
    puts "Got #{count} responses"
  end
end

