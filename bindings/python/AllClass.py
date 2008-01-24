from OWsman import *
from lxml import etree as ElementTree
from lxml import objectify
from StringIO import StringIO


class AllClass(Wsman):
		resource_uri = "http://schemas.dmtf.org/wbem/wscim/1/*"
		classname = ""

		def __init__(self, arg):
			Wsman.__init__(self, arg)
			self.cl = arg

		def Subscribe(self, **param):
			return Wsman.Subscribe(self, self.resource_uri, **param)



