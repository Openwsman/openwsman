# get_class.rb
# Tests an openwsman 2.2 extension: Intrinsic WBEM operations via 'invoke'
#

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'

def show_fault result
  fault = Openwsman::Fault.new result
  STDERR.puts "Fault code #{fault.code}, subcode #{fault.subcode}"
  STDERR.puts "\treason #{fault.reason}"
  STDERR.puts "\tdetail #{fault.detail}"
end

def show_error client
  STDERR.puts "Client failed"
  STDERR.puts "\tResult code #{client.response_code}, Fault: #{client.fault_string}"
end

def fault? client, result
  if result
    if result.fault?
      show_fault result
      true
    end
  else
    show_error client
    true
  end
end

class WsmanTest < Test::Unit::TestCase
  def test_get_class
    Openwsman::debug = 1
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
#    options.set_dump_request
    
    method = Openwsman::CIM_ACTION_GET_CLASS
    uri = Openwsman::XML_NS_CIM_INTRINSIC + "/CIM_ComputerSystem"
#    uri = Openwsman::CIM_ALL_AVAILABLE_CLASSES
    
#    options.properties = { "Prefix" => "Linux" }

    result = client.invoke( options, uri, method )
    fault? client, result
    assert result
#    print result
    unless result.fault?
      node = result.body[method]
      name = node["name"]
      qualifiers = node["qualifiers"]
      properties = node["properties"]
      puts "Class #{name} with #{qualifiers.size} Qualifiers and #{properties.size} Properties"
      puts result.to_xml
      qualifiers.each { |q| puts q.to_xml }
      properties.each { |p| puts p.to_xml }
    else
      puts "--Fault--"
    end
  end
end

