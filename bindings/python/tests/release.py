import unittest

import sys
# automake build dir
sys.path.insert(0, '..')
sys.path.insert(0, '../.libs')
# cmake build dir
sys.path.insert(0, '../../../build/bindings/python')

from pywsman import *



class TestSequenceFunctions(unittest.TestCase):

	def test_enum_release(self):
#		set_debug(1) # enable to print logging to stderr
		client = Client( "http://wsman:secret@localhost:5985/wsman" )
		client.transport().set_auth_method(BASIC_AUTH_STR) # Windows winrm needs this
		options = ClientOptions()
#		uri = "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_OperatingSystem"
		uri = "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
#		options.set_dump_request() # enable to print SOAP request to stdout
		doc = client.enumerate( options, None, uri)
		root = doc.root()
		assert root is not None
		context = root.find(XML_NS_ENUMERATION, "EnumerationContext" )
		doc = client.release( options, uri, context.__str__())
		assert doc is not None
		root = doc.root()
		assert root is not None
		resp = root.find(XML_NS_ADDRESSING, "Action" )
		self.assertEquals( resp.__str__(), XML_NS_ENUMERATION + '/' +  WSENUM_RELEASE_RESP )
		print "Action %s" % resp


if __name__ == '__main__':
    unittest.main()
