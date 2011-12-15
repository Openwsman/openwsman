# win32_registry.rb
#
# Accessing Windows Registry through WS-Man
#
# References:
#   VisualBasic demo script:
#     http://gallery.technet.microsoft.com/ScriptCenter/en-us/b994f262-be39-448d-ab62-edbbeacf2b0f
#   StdRegProv class:
#     http://msdn.microsoft.com/en-us/library/aa393664(v=vs.85).aspx
#   EnumKey Method of the StdRegProv Class:
#     http://msdn.microsoft.com/en-us/library/aa390387(v=vs.85).aspx
#

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

    # Class uri
    # note the root/default namespace (not root/cimv2)
    #
    uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/default/StdRegProv"

    # Selectors are for key/value pairs identifying instances
    #
    # StdRegProv is a Singleton, no selectors needed
    #
    # options.add_selector( "key", value )

    
    # mapping for Registry 'root' pointers
    hkeys = {
      :HKEY_CLASSES_ROOT   => 2147483648, #(0x80000000)
      :HKEY_CURRENT_USER   => 2147483649, #(0x80000001)    
      :HKEY_LOCAL_MACHINE  => 2147483650, #(0x80000002)
      :HKEY_USERS          => 2147483651, #(0x80000003)
      :HKEY_CURRENT_CONFIG => 2147483653, #(0x80000005)
      :HKEY_DYN_DATA       => 2147483654 #(0x80000006)
    }

    # Properties add method parameters
    # (Marked with [in] in method definitions)
    #
    # The hDefKey is optional and defaults to 2147483650 (HKEY_LOCAL_MACHINE)
    # The sSubKeyName is the path name within the Registry
    #
    
    options.properties = {
      "hDefKey" => hkeys[:HKEY_LOCAL_MACHINE].to_s,
      "sSubKeyName" =>  "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"
    }
    
    # Name of method invoked on the class (resp. instrance)
    method = "EnumKey"
    result = client.invoke( options, uri, method )
    assert result
    result.sNames.parent.each do |node|
      puts "#{node.name} #{node.text}"
    end
  end
end

