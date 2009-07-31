#
# test end point reference
#

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'

class LoadTest < Test::Unit::TestCase
  def test_epr
    epr = Openwsman::EndPointReference.new "uri", "namespace"
    assert epr
    
  end
end

