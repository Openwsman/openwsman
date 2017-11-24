#
# testing EndPointReference
#
import unittest
import sys
# automake build dir
sys.path.insert(0, '..')
sys.path.insert(0, '../.libs')
# cmake build dir
sys.path.insert(0, '../../../build/bindings/python')

from pywsman import *

class TestEndPointReference(unittest.TestCase):
    def test_endpoint_reference(self):
        epr = EndPointReference("http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Service?Name=Windows&Vendor=Microsoft")
        assert epr is not None
        print( epr.to_xml() )
        s1 = epr.selector("Name")
        assert s1 is not None
        s2 = epr.selector("Vendor")
        assert s2 is not None
        names = epr.selector_names()
        print( "Names: ", names )

if __name__ == '__main__':
    unittest.main()
