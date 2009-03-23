# cim_computer_system.rb
#  enumerate/pull/release for CIM_SystemConfiguration

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'rexml/document'
require 'rbwsman'
require '_client'

class WsmanTest < Test::Unit::TestCase
  def test_client
    client = Client.open
    assert client
    options = Rbwsman::ClientOptions.new
    assert options
#    options.flags = WsMan::ClientOption::DUMP_REQUEST
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_SystemConfiguration"
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
	result = client.pull( uri, context, options )
	puts "Response ##{count}"
#	puts "### #{result}"
	result.body.PullResponse.Items.child.each_child { |child|
	    puts "\t#{child.name} = #{child.text}"
	}
	context = result.context
    end
    client.release( uri, context, options ) if context
    puts "Got #{count} responses"
  end
end

