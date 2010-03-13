# fault.rb
#
# Test WsManFault bindings
#

$:.unshift File.join(File.dirname(__FILE__), "..","..","..","build","bindings","ruby")
$:.unshift File.join(File.dirname(__FILE__), "..",".libs")
$:.unshift File.join(File.dirname(__FILE__), "..")

require 'test/unit'
require 'openwsman/openwsman'

class WsmanTest < Test::Unit::TestCase
  def test_fault
    status = Openwsman::Status.new
    puts "Status #{status}"
    code = Openwsman::SOAP_FAULT_MUSTUNDERSTAND
    detail = Openwsman::WSMAN_DETAIL_EXPIRED
    msg = "Testing fault handling"
    status.code = code
    puts "Status.code #{status}"
    status.detail = detail
    puts "Status.detail #{status}"
    status.msg = msg
    puts "Status.msg #{status}"
    
    doc = Openwsman::XmlDoc.new "FaultDoc", "namespace"
    puts "Doc #{doc}"
    fault = doc.generate_fault status
    puts "Fault #{fault}"
    
    assert fault.fault?
    assert_equal code, fault.code
    assert_equal detail, fault.detail
    assert_equal msg, fault.msg
    
  end
end

