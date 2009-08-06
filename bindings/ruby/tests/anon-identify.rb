# test identify action

$:.unshift "../../../build/bindings/ruby"
$:.unshift "../.libs"

require 'test/unit'
require 'openwsman'
require 'auth-callback'

class IdentifyTest < Test::Unit::TestCase
  def test_identify
    client = Openwsman::Client.new( "http://wsman:secret@localhost:5985/wsman-anon/identify" )
    assert client
    options = Openwsman::ClientOptions.new
    assert options
    doc = client.identify( options )
    assert doc
    root = doc.root
    assert root
    prot_version = root.find( Openwsman::XML_NS_WSMAN_ID, "ProtocolVersion" )
    prod_vendor = root.find( Openwsman::XML_NS_WSMAN_ID, "ProductVendor" )
    prod_version = root.find( Openwsman::XML_NS_WSMAN_ID, "ProductVersion" )
    puts "Protocol #{prot_version}, Vendor #{prod_vendor}, Version #{prod_version}"
  end
end

