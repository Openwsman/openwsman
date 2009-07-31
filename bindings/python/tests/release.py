import unittest
from pywsman import *



class TestSequenceFunctions(unittest.TestCase):

	def test_enum_release(self):
		client = Client( "http://wsman:secret@localhost:8889/wsman" )
		options = ClientOptions()
		doc = client.enumerate( options , None, "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem")
		root = doc.root()
		assert root is not None
		context = root.find(XML_NS_ENUMERATION, "EnumerationContext" )
		doc = client.release( options , "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", context.text())
		assert doc is not None
		root = doc.root()
		assert root is not None
		resp = root.find(XML_NS_ADDRESSING, "Action" )
		self.assertEquals( resp.text(), XML_NS_ENUMERATION + '/' +  WSENUM_RELEASE_RESP )



if __name__ == '__main__':
    unittest.main()
