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
        options = ClientOptions()
        assert options is not None
        options.add_selector("foo", "bar")

if __name__ == '__main__':
    unittest.main()
    