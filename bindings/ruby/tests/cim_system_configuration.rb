# cim_computer_system.rb
#  enumerate/pull/release for CIM_SystemConfiguration

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
    classname = "CIM_ManagedElement"
    uri = "#{Openwsman.epr_prefix_for(classname)}/#{classname}"
    result = client.enumerate( options, nil, uri )
    assert result
#    puts result

    count = 0
    context = result.context
    while (context)
      count += 1
      result = client.pull( options, nil, uri, context )
#      puts "Response ##{count}"
#      puts "### #{result.to_xml}"
      result.body.PullResponse.Items.each do |child|
	puts "#{child.name}"
        child.each do |prop|
          puts "\t#{prop.name} = #{prop.text}"
        end
      end
      context = result.context
    end
    client.release( options, uri, context ) if context
    puts "Got #{count} responses"
  end
end

