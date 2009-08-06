# test identify action

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman/openwsman'

class IdentifyTest < Test::Unit::TestCase
  def test_identify
#   Openwsman::debug = 1
    client = Openwsman::Client.new( "http://wsman:secret@localhost:5985/wsman" )
    assert client
    client.transport.auth_method = Openwsman::BASIC_AUTH_STR
    options = Openwsman::ClientOptions.new
    assert options
    doc = client.identify( options )
    assert doc
    root = doc.root
    assert root
#
# root.find is the 'clumsy' way
#    prot_version = root.find( Openwsman::XML_NS_WSMAN_ID, "ProtocolVersion" )
#    prod_vendor = root.find( Openwsman::XML_NS_WSMAN_ID, "ProductVendor" )
#    prod_version = root.find( Openwsman::XML_NS_WSMAN_ID, "ProductVersion" )
    
    # Ruby allows to catch 'method_missing' making it much more elegant:
    prot_version = root.ProtocolVersion
    prod_vendor = root.ProductVendor
    prod_version = root.ProductVersion

    puts "Protocol #{prot_version}, Vendor #{prod_vendor}, Version #{prod_version}"
    
  end
end

