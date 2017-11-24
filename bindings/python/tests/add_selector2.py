import unittest
import sys
# automake build dir
sys.path.insert(0, '..')
sys.path.insert(0, '../.libs')
# cmake build dir
sys.path.insert(0, '../../../build/bindings/python')

from pywsman import *

class TestAddSelector(unittest.TestCase):
    def test_add_selector(self):
        client = Client( "http://wsman:secret@localhost:5985/wsman" )
        assert client is not None
        client.transport().set_auth_method(BASIC_AUTH_STR) # Windows winrm needs this
        options = ClientOptions()
        assert options is not None
        options.set_dump_request()
        uri_query = "k1=v1,k2=v2,k3=v3,k4=v4"
        for selector_str in uri_query.split(","):
            (key_asdf, value_asdf) = selector_str.split("=")
            assert key_asdf is not None
            assert value_asdf is not None
            print( "Calling add_selector(%r, %r)" % (key_asdf, value_asdf) )
            options.add_selector(str(key_asdf), str(value_asdf))
        dummy = client.invoke(options, "http://uri", "method", XmlDoc("dummy doc"))

if __name__ == '__main__':
    unittest.main()
