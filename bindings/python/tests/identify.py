import unittest

import sys
# automake build dir
sys.path.insert(0, '..')
sys.path.insert(0, '../.libs')
# cmake build dir
sys.path.insert(0, '../../../build/bindings/python')

from pywsman import *



class TestSequenceFunctions(unittest.TestCase):

   def test_identify(self):
#     set_debug(1) # enable to print logging to stderr
      client = Client( "http://wsman:secret@localhost:5985/wsman" )
      assert client is not None
      client.transport().set_auth_method(BASIC_AUTH_STR) # Windows winrm needs this
      options = ClientOptions()
      assert options is not None
#     options.set_dump_request() # enable to print SOAP request to stdout
      doc = client.identify( options )
      assert doc is not None
#      print "Document [%s]" % doc
      root = doc.root()
      assert root is not None
#      print "Root [%s]" % root
      prot_version = root.find( XML_NS_WSMAN_ID, "ProtocolVersion" )
      prod_vendor = root.find( XML_NS_WSMAN_ID, "ProductVendor" )
      prod_version = root.find( XML_NS_WSMAN_ID, "ProductVersion" )
      print "Protocol %s, Vendor %s, Version %s"  %( prot_version, prod_vendor, prod_version )


if __name__ == '__main__':
    unittest.main()
