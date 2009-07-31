import unittest
from pywsman import *



class TestSequenceFunctions(unittest.TestCase):

	def test_identify(self):
		client = Client( "http://wsman:secret@localhost:8889/wsman" )
		assert client is not None
		options = ClientOptions()
		assert options is not None
		doc = client.identify( options )
		assert doc is not None
		root = doc.root()
		assert root is not None
		prot_version = root.find( XML_NS_WSMAN_ID, "ProtocolVersion" )
		prod_vendor = root.find( XML_NS_WSMAN_ID, "ProductVendor" )
		prod_version = root.find(XML_NS_WSMAN_ID, "ProductVersion" )
		print "Protocol %s, Vendor %s, Version %s"  %( prot_version.text(), prod_vendor.text(), prod_version.text() )


if __name__ == '__main__':
    unittest.main()
