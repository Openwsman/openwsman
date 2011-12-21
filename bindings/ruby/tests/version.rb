# test versioning

require 'test/unit'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'

class VersionTest < Test::Unit::TestCase
  def test_version
    assert Openwsman::OPENWSMAN_VERSION
    assert Openwsman::OPENWSMAN_MAJOR >= 0
    assert Openwsman::OPENWSMAN_MINOR >= 0
    assert Openwsman::OPENWSMAN_PATCH >= 0
  end
end
