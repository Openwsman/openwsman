# test loading of pywsman
import sys

# automake build dir
sys.path.insert(0, '..')
# cmake build dir
sys.path.insert(0, '../../../build/bindings/python')

from pywsman import *
