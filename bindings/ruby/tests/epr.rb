#
# test end point reference
#

require 'test/unit'
require '_loadpath'
require 'openwsman'

class LoadTest < Test::Unit::TestCase
  def test_epr
    epr = Openwsman::EndPointReference.new "uri", "namespace"
    assert epr
    
  end
end

