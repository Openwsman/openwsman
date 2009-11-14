import unittest
from pywsman import *



class TestSequenceFunctions(unittest.TestCase):

	def test_enum(self):
		client = Client( "http://wsman:secret@localhost:5985/wsman" )
#		client.transport().set_auth_method(BASIC_AUTH_STR) # Windows winrm needs this
		options = ClientOptions()
		doc = client.enumerate( options , None, "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem")
		assert doc is not None
		root = doc.root()
		assert root is not None
		context = root.find(XML_NS_ENUMERATION, "EnumerationContext" )
		print "Context: %s" % context.text()
	def test_enum_and_pull(self):
		client = Client( "http://wsman:secret@localhost:5985/wsman" )
#		client.transport().set_auth_method(BASIC_AUTH_STR) # Windows winrm needs this
		options = ClientOptions()
		doc = client.enumerate( options , None, "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem")
		root = doc.root()
		assert root is not None
		context = root.find(XML_NS_ENUMERATION, "EnumerationContext" )
		doc = client.pull( options , None, "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", context.text())
		assert doc is not None
		root = doc.root()
		assert root is not None
		pullresp = root.find(XML_NS_ENUMERATION, "PullResponse" )
		assert pullresp is not None

if __name__ == '__main__':
    unittest.main()
