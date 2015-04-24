# cim_process.rb
#  enumerate/pull/release for CIM_Process
#
#

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require File.join(File.dirname(__FILE__), '_client')

class WsmanTest < Test::Unit::TestCase
  def get_owner client, process
    options = Openwsman::ClientOptions.new
    # WMI just needs the Handle
    unless process.name == "Win32_Service"
      options.add_selector( "CSCreationClassName", process.CSCreationClassName )
      options.add_selector( "CSName", process.CSName )
      options.add_selector( "OSCreationClassName", process.OSCreationClassName )
      options.add_selector( "OSName", process.OSName )
      options.add_selector( "CreationClassName", process.CreationClassName )
    end
    options.add_selector( "Handle", process.Handle )
    uri = process.ns
    
    method = "GetOwner"
    result = client.invoke( options, uri, method )

  end

  def test_client
    client = Client.open
    assert client
    options = Openwsman::ClientOptions.new
    assert options
    options.flags = Openwsman::FLAG_ENUMERATION_OPTIMIZATION
    options.max_elements = 999
#    options.set_dump_request

#
# see http://msdn2.microsoft.com/en-us/library/aa386179.aspx for a list of CIM classes
#   the Win32 classes are derived from
#
    uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_Process"

    result = client.enumerate( options, nil, uri )
    if result.nil?
      puts "client.enumerate failed with #{client.response_code}"
      puts "#{client.last_error}:#{client.fault_string}"
    end
    assert result
    if result.fault?
      fault = Openwsman::Fault.new result
      puts "Enumerate returned fault"
      puts "Fault code #{fault.code}, subcode #{fault.subcode}"
      puts "\treason #{fault.reason}"
      puts "\tdetail #{fault.detail}"
            
      exit 1
    end

#    puts "Result #{result.to_xml}"
    results = 0
    faults = 0
    context = nil
 
    printf("%-20s %-10s %-10s  %s\n", "User", "PID", "VSZ", "Command");
    puts "-------------------------------------------------------------"
    
    context = result.context

    result.body.EnumerateResponse.Items.each do |node|

      results += 1
    
#    node = body.child( 0, Openwsman::NS_ENUMERATION, "PullResponse" );
#    node = node.child( 0, Openwsman::NS_ENUMERATION, "Items" );
#    node = node.child( 0, uri, "Win32_Service" );

#    name = node.child( 0, uri, "Name" ).text;
#    state = node.child( 0, uri, "State" ).text;

      if node.name == "Win32_Service"
	caption = node["Caption"]
	handle = node["Handle"]
	virtual_size = node["VirtualSize"]
	proc_id = node["ProcessId"]
	cmd = node["ExecutablePath"]
      else
	caption = node["Caption"]
	handle = node["Handle"]
	virtual_size = 0
	proc_id = handle
	cmd = node["Name"]
      end
      user = ""

      if node.name == "Win32_Service"
	ires = get_owner client, node
	if ires
	  b = ires.body
	  output = b.GetOwner_OUTPUT
	  output.each do |child|
	    text = child.text
	    if child.name == "User"
	      user = text
	    end
	  end if output
	end
      end
      vsz =  (virtual_size.to_s.to_i / (1024 * 1024 ) ).to_f
      
      printf("%-20s %-10s %-5.0f  %s\n", user, proc_id, vsz, cmd);

    end

    client.release( options, uri, context ) if context
    puts "Context released, #{results} results, #{faults} faults"
  end
end

