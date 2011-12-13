# fault.rb
#
# Test WsManFault bindings
#

require 'test/unit'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'

class WsmanTest < Test::Unit::TestCase
  def test_fault
    status = Openwsman::Status.new
    puts "Status #{status}"
    code = Openwsman::SOAP_FAULT_MUSTUNDERSTAND
    detail = Openwsman::WSMAN_DETAIL_EXPIRED
    msg = "Testing fault handling"
    status.code = code
    puts "Status.code #{status.code}"
    status.detail = detail
    puts "Status.detail #{status.detail}"
#    status.msg = msg
    puts "Status.msg '#{status.msg}'"
    
    doc = Openwsman::XmlDoc.new "FaultDoc", "namespace"
    puts "Doc #{doc}"
    faultdoc = doc.generate_fault status
    puts "Fault #{faultdoc}"
    assert faultdoc.fault?
    
    fault = Openwsman::Fault.new faultdoc
    
    puts "Code '#{fault.code}'"
    puts "Subcode '#{fault.subcode}'"
    puts "Reason '#{fault.reason}'"
    puts "Detail '#{fault.detail}'"
    
  end
end

