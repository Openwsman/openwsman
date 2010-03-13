# fault.rb
#
# Test WsManFault bindings
#

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"
$:.unshift ".."

require 'test/unit'
require 'openwsman/openwsman'

class WsmanTest < Test::Unit::TestCase
  def test_fault
    status = Openwsman::Status.new
    code = 42
    detail = 4711
    msg = "Testing fault handling"
    status.code = code
    status.detail = detail
    status.msg = msg
    
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

