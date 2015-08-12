# invoke_erb.rb
#
# Invoke with EndPointReference
# winrm Command :
#
# winrm invoke RealizePlannedSystem
#  http://schemas.microsoft.com/wbem/wsman/1/wmi/root/virtualization/v2/Msvm_VirtualSystemManagementService
#   ?CreationClassName=Msvm_VirtualSystemManagementService
#   +Name=vmms
#   +SystemCreationClassName=Msvm_ComputerSystem
#   +SystemName=hyperv06
#  -file:input.xml
#  -r:<address>:<port>
#  -a:Basic
#  -u:Administrator
#  -p:<password>
#  -SkipCAcheck
#  -SkipCNcheck
#
# Input.xml :
# <p:RealizePlannedSystem_INPUT xmlns:p="http://schemas.microsoft.com/wbem/wsman/1/wmi/root/virtualization/v2/Msvm_VirtualSystemManagementService">
#  <p:PlannedSystem>
#   <a:Address xmlns:a="http://schemas.xmlsoap.org/ws/2004/08/addressing">http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address>
#   <a:ReferenceParameters xmlns:a="http://schemas.xmlsoap.org/ws/2004/08/addressing" xmlns:w="http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd">
#    <w:ResourceURI>http://schemas.microsoft.com/wbem/wsman/1/wmi/root/virtualization/v2/Msvm_PlannedComputerSystem</w:ResourceURI>
#    <w:SelectorSet>
#     <w:Selector Name="CreationClassName">Msvm_PlannedComputerSystem</w:Selector>
#     <w:Selector Name="Name">8FF5E6F3-A141-465B-83ED-3BFC0F59F7EF</w:Selector>
#    </w:SelectorSet>
#   </a:ReferenceParameters>
#  </p:PlannedSystem>
# </p:RealizePlannedSystem_INPUT>
#
# Request
#<s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" xmlns:a="http://schemas.xmlsoap.org/ws/2004/08/addressing" xmlns:w="http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd" xmlns:p="http://schemas.microsoft.com/wbem/wsman/1/wsman.xsd">
#  <s:Header>
#    <a:To>http://10.120.4.11:5985/wsman</a:To>
#    <w:ResourceURI s:mustUnderstand="true">http://schemas.microsoft.com/wbem/wsman/1/wmi/root/virtualization/v2/Msvm_VirtualSystemManagementService</w:ResourceURI>
#    <a:ReplyTo>
#      <a:Address s:mustUnderstand="true">http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address>
#    </a:ReplyTo>
#    <a:Action s:mustUnderstand="true">http://schemas.microsoft.com/wbem/wsman/1/wmi/root/virtualization/v2/Msvm_VirtualSystemManagementService/RealizePlannedSystem</a:Action>
#    <w:MaxEnvelopeSize s:mustUnderstand="true">512000</w:MaxEnvelopeSize>
#    <a:MessageID>uuid:AC1C5EDE-BC24-4F46-9A15-78AAF883F2E0</a:MessageID>
#    <w:Locale xml:lang="en-US" s:mustUnderstand="false"/>
#    <p:DataLocale xml:lang="en-US" s:mustUnderstand="false"/>
#    <p:SessionId s:mustUnderstand="false">uuid:8FAE7733-6B3B-4DB8-83E8-37C4537FCD9D</p:SessionId>
#    <p:OperationID s:mustUnderstand="false">uuid:0226A933-16BB-4537-B0E6-060B962FC19D</p:OperationID>
#    <p:SequenceId s:mustUnderstand="false">1</p:SequenceId>
#    <w:SelectorSet>
#      <w:Selector Name="CreationClassName">Msvm_VirtualSystemManagementService</w:Selector>
#      <w:Selector Name="Name">vmms</w:Selector>
#      <w:Selector Name="SystemCreationClassName">Msvm_ComputerSystem</w:Selector>
#      <w:Selector Name="SystemName">hyperv06</w:Selector>
#    </w:SelectorSet>
#    <w:OperationTimeout>PT60.000S</w:OperationTimeout>
#  </s:Header>
#  <s:Body>
#    <p:RealizePlannedSystem_INPUT xmlns:p="http://schemas.microsoft.com/wbem/wsman/1/wmi/root/virtualization/v2/Msvm_VirtualSystemManagementService">
#      <p:PlannedSystem>
#        <a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address>
#        <a:ReferenceParameters>
#          <w:ResourceURI>http://schemas.microsoft.com/wbem/wsman/1/wmi/root/virtualization/v2/Msvm_PlannedComputerSystem</w:ResourceURI>
#          <w:SelectorSet>
#            <w:Selector Name="CreationClassName">Msvm_PlannedComputerSystem</w:Selector>
#            <w:Selector Name="Name">8FF5E6F3-A141-465B-83ED-3BFC0F59F7EF</w:Selector>
#          </w:SelectorSet>
#        </a:ReferenceParameters>
#      </p:PlannedSystem>
#    </p:RealizePlannedSystem_INPUT>
#  </s:Body>
#</s:Envelope>

require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'

client = Client.open
options = Openwsman::ClientOptions.new
options.set_dump_request

uri = Openwsman.epr_uri_for("root/virtualization/v2", "Msvm_VirtualSystemManagementService")

options.add_selector("CreationClassName", "Msvm_VirtualSystemManagementService")
options.add_selector("Name", "vmms")
options.add_selector("SystemCreationClassName", "Msvm_ComputerSystem")
options.add_selector("SystemName", "hyperv06")

method = "RealizePlannedSystem"

epr = Openwsman::EndPointReference.new(Openwsman.epr_uri_for("root/virtualization/v2", "Msvm_PlannedComputerSystem"))
epr.add_selector("CreationClassName", "Msvm_PlannedComputerSystem")
epr.add_selector("Name", "8FF5E6F3-A141-465B-83ED-3BFC0F59F7EF")

# puts "EPR:\n#{epr.to_xml}"

options.add_property("PlannedSystem", epr)
options.add_property("Two", "two")
options.add_property("One", "one")

result = client.invoke( options, uri, method )
fault? client, result
