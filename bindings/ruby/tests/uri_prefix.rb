#
# test end point reference uri prefix
#

require 'test/unit'
require File.expand_path(File.join(File.dirname(__FILE__),'_loadpath'))
require 'openwsman'

class UriPrefixTest < Test::Unit::TestCase
  def test_uri_prefix
    { "CIM_Class" => "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2",
      "PRS_Class" => "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2",
      "Win32_Class" => "http://schemas.microsoft.com/wbem/wsman/1/wmi",
      "OpenWBEM_Class" => "http://schema.openwbem.org/wbem/wscim/1/cim-schema/2",
      "Linux_Class" => "http://sblim.sf.net/wbem/wscim/1/cim-schema/2",
      "OMC_Class" => "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2",
      "PG_Class" => "http://schema.openpegasus.org/wbem/wscim/1/cim-schema/2",
      "AMT_Class" => "http://intel.com/wbem/wscim/1/amt-schema/1",
      "IPS_Class" => "http://intel.com/wbem/wscim/1/ips-schema/1" }.each do |k,v|
      assert_equal v, Openwsman::uri_prefix(k)
    end
  end
end
