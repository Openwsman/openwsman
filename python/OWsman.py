import OpenWSMan
from lxml import etree as ElementTree
from lxml import objectify
from StringIO import StringIO

NS_SOAP_12 = "http://www.w3.org/2003/05/soap-envelope"
NS_WS_ADDRESSING = "http://schemas.xmlsoap.org/ws/2004/08/addressing"
NS_WS_MANAGEMENT = "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
NS_WS_ENUMERATION = "http://schemas.xmlsoap.org/ws/2004/09/enumeration"
NS_WS_TRANSFER = "http://schemas.xmlsoap.org/ws/2004/09/transfer"
FABRIKAM = "http://fabrikam.org/fabrikam"

WSM_ASSOCIATION_FILTER_DIALECT  =  "http://schemas.dmtf.org/wbem/wsman/1/cimbinding/associationFilter"
ALL_CLASS_URI =  "http://schemas.dmtf.org/wbem/wscim/1/*"

namespaces =  { "s": NS_SOAP_12 ,"wsa": NS_WS_ADDRESSING , "wsen": NS_WS_ENUMERATION,
		            "wxf": NS_WS_TRANSFER,"wsman": NS_WS_MANAGEMENT , "fabrikam": FABRIKAM}


class EndPointReference:

	def __init__(self):
		self._ReferenceParameters = None
		self._Address = None
		self._ReferenceProperties = None
    	def setAddress(self, value):
        	self._Address = value

    	def setReferenceProperties(self, value):
        	self._ReferenceProperties = value

    	def setReferenceParameters(self, value):
        	self._ReferenceParameters = value


class ReferenceParameters:
    	def setResourceURI(self, value):
        	self._resource_uri = value

    	def setSelectorSet(self, value):
        	self._selector_set = value



class WsmanClient:
	def __init__(self):
		self.uri = None
		self.hostname = None
		self.port = None
		self.path = None
		self.scheme = None
		self.username = None
		self.password = None
	def create(self, hostname, port , path, scheme, username, password):
		self.hdl = OpenWSMan.wsmc_create(hostname, port , path, scheme, username, password)
	def create_from_uri(self, uri):
		self.hdl = OpenWSMan.wsmc_create_from_uri(uri)


class Wsman:
	namespaces =  { "s": NS_SOAP_12 ,"wsa": NS_WS_ADDRESSING , "wsen": NS_WS_ENUMERATION,
		            "wxf": NS_WS_TRANSFER,"wsman": NS_WS_MANAGEMENT , "fabrikam": FABRIKAM}
	encoding = 'UTF-8';
	def __init__(self, arg):
		self.cl = arg
	def GetContext(self, xmlresp):
		context = xmlresp.xpath("/s:Envelope/s:Body/wsen:EnumerateResponse/wsen:EnumerationContext", self.namespaces )
		return context
	def GetPullContext(self, xmlresp):
		context = xmlresp.xpath("/s:Envelope/s:Body/wsen:PullResponse/wsen:EnumerationContext", self.namespaces )
		return context
	def GetItems(self, xmlresp):
		items = xmlresp.xpath("/s:Envelope/s:Body/wsen:PullResponse/wsen:Items/*", self.namespaces )
		return items

	def Identify(self):
		op = OpenWSMan.wsmc_options_init()
		resp = OpenWSMan._identify(self.cl.hdl, op , self.encoding)
		xmlresp = ElementTree.parse(StringIO(resp))
		return xmlresp
	def Enumerate(self, ruri, **param):
		op = OpenWSMan.wsmc_options_init()
		for x in param.keys():
			if x == "filter":
				OpenWSMan.wsmc_set_filter(param[x], op)
			elif x == "dialect":
				OpenWSMan.wsmc_set_dialect(param[x], op)
			elif x == "assoc" and param[x]:
				OpenWSMan._set_assoc_filter()
		items = []
		resp = OpenWSMan._enumerate(self.cl.hdl,  ruri, op,  self.encoding)
		xmlresp = ElementTree.parse(StringIO(resp))
		context = self.GetContext(xmlresp)
		while context:
			resp = OpenWSMan._pull(self.cl.hdl,  ruri, op, context[0].text , self.encoding)
			xmlresp = ElementTree.parse(StringIO(resp))
			items.append(self.GetItems(xmlresp))
			context = self.GetPullContext(xmlresp)

		return items

	# FIXME
	def Associations(self, object):
		path = obgect.getpath()
		items = Wsman.Enumerate(self, allclass, dialect=WSM_ASSOCIATION_FILTER_DIALECT, assoc=True , filter=path)
		return items

	def Get(self, ruri, selectors):
		op = OpenWSMan.wsmc_options_init()
		for x in selectors.keys():
			OpenWSMan.wsmc_add_selector(op, x, selectors[x])
		resp = OpenWSMan._get(self.cl.hdl, ruri, op, self.encoding)
		return resp
	def get_from_epr(self, epr):
		resp = Wsman.Get(self, epr._ReferenceParameters._resource_uri, epr._ReferenceParameters._selector_set)
		env = ElementTree.parse(StringIO(resp))
		self.xmlresp = env.xpath("//s:Body/*", self.namespaces)[0]
		self._set()

	def get_epr(self, xmlresp , prop):
		for el in xmlresp.getiterator():
			elx = el.prefix + ":" + prop
			res = xmlresp.xpath(elx , el.nsmap )
			nsm = el.nsmap
			if isinstance(res, list) and len(res) > 0:
				epr = EndPointReference()
				param = ReferenceParameters()


				addr = res[0].xpath("//" + elx + "/wsa:Address", nsm)[0].text
				ruri = res[0].xpath("//" + elx + "/wsa:ReferenceParameters/wsman:ResourceURI", nsm)[0].text
				param.setResourceURI(ruri)
				selectors = res[0].xpath("//" + elx + "/wsa:ReferenceParameters/wsman:SelectorSet/*", nsm)
				selectorset = { }
				for s in selectors:
					selectorset[s.get("Name")] = s.text


				param.setSelectorSet(selectorset)
				epr.setAddress(addr)
				epr.setReferenceParameters(param)
				return epr
			else:
				return None
