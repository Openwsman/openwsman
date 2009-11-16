import unittest
from pywsman import *



class TestSequenceFunctions(unittest.TestCase):
	def test_client_constructor_uri_simple(self):
		client = Client("http://localhost")
		self.assertEqual(client.scheme() , "http")
		self.assertEqual(client.host() , "localhost")
	def test_client_constructor_uri(self):
		client = Client( "https://wsman:secret@localhost:5985/wsman" )
		assert client is not None
		self.assertEqual(client.scheme() , "https" )
		self.assertEqual(client.user() , "wsman" )
		self.assertEqual(client.password() , "secret" )
		self.assertEqual(client.host() , "localhost" )
		self.assertEqual(client.port() , 5985 )
		self.assertEqual(client.path() , "/wsman" )

	def test_client_constructor_full(self):
		client = Client( "localhost", 5985, "/wsman", "http", "wsman", "secret" )
		assert client is not None
		self.assertEqual(client.scheme() , "http" )
		self.assertEqual(client.user() , "wsman" )
		self.assertEqual(client.password() , "secret")
		self.assertEqual(client.host() , "localhost" )
		self.assertEqual(client.port() ,5985 )
		self.assertEqual(client.path() ,"/wsman")

	def test_client_options_constructor(self):
		options = ClientOptions()
		assert options is not None

	def test_identify(self):
		client = Client( "http://wsman:secret@localhost:5985/wsman" )
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
		print "Protocol %s, Vendor %s, Version %s"  %( prot_version, prod_vendor, prod_version )


if __name__ == '__main__':
    unittest.main()
