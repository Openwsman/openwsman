#
# test end point reference
#

require 'test/unit'
require File.expand_path(File.join(File.dirname(__FILE__),'_loadpath'))
require 'openwsman'

class EprTest < Test::Unit::TestCase
  def test_epr_create_from_uri_and_namespace
    epr = Openwsman::EndPointReference.new "uri", "namespace"
    assert epr
  end

  def test_epr_create_from_uri
    epr = Openwsman::EndPointReference.new "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter?Name=OperatingSystemFilter0&CreationClassName=CIM_IndicationFilter&SystemName=localhost.localdomain&SystemCreationClassName=CIM_ComputerSystem"
    assert epr
    
    assert_equal "CIM_IndicationFilter", epr.classname
    assert_equal "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2", epr.prefix
    assert_equal "", epr.namespace
  end

  def test_epr_create_from_xml
    doc = Openwsman::create_soap_envelope
    body = doc.body
    # <n:Items>
    #   <a:EndpointReference>
    #     <a:Address>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</a:Address>
    #     <a:ReferenceParameters>
    #       <w:ResourceURI>http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service</w:ResourceURI>
    #       <w:SelectorSet>
    #         <w:Selector Name="Name">AeLookupSvc</w:Selector>
    #       </w:SelectorSet>
    #     </a:ReferenceParameters>
    #   </a:EndpointReference>
    # </n:Items>
                                                                                                          
    items = body.add(Openwsman::XML_NS_ENUMERATION, Openwsman::WSENUM_ITEMS)
    epr = items.add(Openwsman::XML_NS_ADDRESSING, Openwsman::WSA_EPR)
    epr.add(Openwsman::XML_NS_ADDRESSING, Openwsman::WSA_ADDRESS, Openwsman::WSA_TO_ANONYMOUS)
    ref = epr.add(Openwsman::XML_NS_ADDRESSING, Openwsman::WSA_REFERENCE_PARAMETERS)
    ref.add(Openwsman::XML_NS_WS_MAN, Openwsman::WSM_RESOURCE_URI, "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2/CIM_IndicationFilter")
    selectors = ref.add(Openwsman::XML_NS_WS_MAN, Openwsman::WSM_SELECTOR_SET)
    { "Name"=>"OperatingSystemFilter0", "CreationClassName"=>"CIM_IndicationFilter",
      "SystemName"=>"localhost.localdomain", "SystemCreationClassName"=>"CIM_ComputerSystem"}.each do |k,v|
      sel = selectors.add(Openwsman::XML_NS_WS_MAN, Openwsman::WSM_SELECTOR, v)
      sel.attr_add(nil, "Name", k)
    end
    
    ref = doc.EndpointReference
    epr = Openwsman::EndPointReference.new ref
    assert epr
    
    epr = Openwsman::EndPointReference.new doc
    assert epr
  end
  
  def test_epr_compare
    epr1 = Openwsman::EndPointReference.new "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service?Name=Windows&Vendor=Microsoft"
    assert epr1
puts "EPR1 #{epr1}"
    # check namespace
    assert_equal "root/cimv2", epr1.namespace

    assert_equal 2, epr1.selector_count
    assert_equal "Windows", epr1.selector("Name")
    assert_equal "Windows", epr1.selector(:Name)
    assert_equal "Microsoft", epr1.selector("Vendor")
    assert_equal "Microsoft", epr1.selector(:Vendor)

    # Check selector_names
    expected = { "Name" => "Windows", "Vendor" => "Microsoft" }
    assert_equal expected.keys.sort, epr1.selector_names.sort

    # Check iterator
    count = 0
    epr1.each do |k,v|
      count += 1;
      expected.delete(k) if expected[k] == v
    end
    assert_equal 2, count
    assert_equal 0, expected.size

    # Check selector shortcut
    assert_equal "Windows", epr1.Name
    assert_equal "Microsoft", epr1.Vendor

    # convert epr1 to xml-string and parse this string
    doc = Openwsman::create_doc_from_string epr1.to_xml
    # generate epr2 from parsed xml
    puts "DOC #{doc.to_xml}"
    epr2 = Openwsman::EndPointReference.new doc
    assert epr2
puts "EPR2 #{epr2}"

    # must be true
    assert epr1.cmp(epr2)
  end
end

