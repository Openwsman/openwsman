#
# testing XmlNode
#
import unittest

from pywsman import *

class TestXmlNode(unittest.TestCase):
    def test_xml_node(self):
        doc = XmlDoc("node")
        assert doc is not None
        root = doc.root()
        root.add( XML_NS_SOAP_1_2, "one", "This is node one" )
        print( "Root:", root.string() )
        node = root.child()
        assert node is not None
        print( "Node:", node.string() )
        print( "Node namespace:", node.ns() )
        print( "Node name:", node.name() )
        print( "Node text:", node.__str__() )
        self.assertEqual( node.name(), "one" )

if __name__ == '__main__':
    unittest.main()
