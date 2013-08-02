# create_shell.rb

#
# Create a remote window shell
#

require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'

def handle_fault result
end

  client = Openwsman::Client.new( "10.120.5.37", 5985, "/wsman", "http", "wsman", "secret")
  client.transport.timeout = 120
  client.transport.auth_method = Openwsman::BASIC_AUTH_STR
  # https
  # client.transport.verify_peer = 0
  # client.transport.verify_host = 0

  options = Openwsman::ClientOptions.new
  options.set_dump_request
  options.timeout = 60 * 1000 # 60 seconds
  uri = "http://schemas.microsoft.com/wbem/wsman/1/windows/shell/cmd"

  service = "Themes"
  options.add_selector( "Name", service )
    
  # instance values
  instance = { "InputStreams" => "stdin", "OutputStreams" => "stdout stderr" }

  namespace = "http://schemas.microsoft.com/wbem/wsman/1/windows/shell"
  data = Openwsman::XmlDoc.new("Shell", namespace)
  root = data.root
  instance.each do |key,value|
    root.add namespace, key, value
  end
    
  s = data.to_xml
  result = client.create( options, uri, s, s.size, "utf-8" )
  # returns something like
  #  <s:Body>
  #    <x:ResourceCreated>
  #      <a:Address>http://10.120.5.37:5985/wsman</a:Address>
  #      <a:ReferenceParameters>
  #        <w:ResourceURI>http://schemas.microsoft.com/wbem/wsman/1/windows/shell/cmd</w:ResourceURI>
  #        <w:SelectorSet>
  #          <w:Selector Name="ShellId">3D5D8879-98EA-49B7-9A33-6842EC0D35D0</w:Selector>
  #        </w:SelectorSet>
  #      </a:ReferenceParameters>
  #    </x:ResourceCreated>
  #  </s:Body>
  handle_fault result if result.fault?

  selector = result.find(nil, "Selector")
  raise "No shell id returned" unless selector
  puts selector

