# exec_cmd.rb
#
# Test Linux_OperatingSystem.execCmd of sblim-cmpi-base
#

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'


def report_fault fault
  puts "Error!"
  puts fault
#  puts "Code #{fault.code}"
#  puts "Subcode #{fault.subcode}"
#  puts "Reason #{fault.reason}"
#  puts "Detail #{fault.detail}" 
end

class WsmanTest < Test::Unit::TestCase
  def test_exec_cmd
    
    Openwsman.debug = -1
    
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
    options.set_dump_request
    
    uri = "http://sblim.sf.net/wbem/wscim/1/cim-schema/2/Linux_OperatingSystem"
#    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_OperatingSystem"

    # enumerate instances
    result = client.enumerate( options, nil, uri )
    if result.fault?
      report_fault result.fault
      exit 1
    end
    
    result = client.pull( options, nil, uri, result.context )
    
    puts "Pull #{result}"
    
    # extract instance key properties
    # and move them to selector options
    
    options.add_selector( "CSCreationClassName", result.CSCreationClassName )
    options.add_selector( "CSName", result.CSName )
    options.add_selector( "CreationClassName", result.CreationClassName )
    options.add_selector( "Name", result.Name )

    method = "execCmd"
    options.properties = { "cmd" => "ls /" }
    result = client.invoke( options, uri, method )
    assert result

    bodychild = result.body.child
    if result.fault?
      report_fault result.fault
      exit 1
    end
    puts "Result code #{client.response_code}"
    puts result
  end
end

