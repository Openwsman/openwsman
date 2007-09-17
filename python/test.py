

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

namespaces =  { "s": NS_SOAP_12 ,"wsa": NS_WS_ADDRESSING , "wsen": NS_WS_ENUMERATION,
		            "wxf": NS_WS_TRANSFER,"wsman": NS_WS_MANAGEMENT , "fabrikam": FABRIKAM}

allclass =  "http://schemas.dmtf.org/wbem/wscim/1/*"

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





class CIM_Processor(Wsman):
	resource_uri =  "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_Processor"
	classname =  "CIM_Processor"
	def __init__(self, arg):
		Wsman.__init__(self, arg)
		self.cl = arg

		self.ElementName = None
		self.CreationClassName = None
		self.DeviceID = None
		self.xmlresp = None
	def get_value(self, prop):
		element = None
		for el in self.xmlresp.getiterator():
			res = self.xmlresp.xpath(el.prefix + ":" + prop , el.nsmap )
			if isinstance(res, list):
				return res[0].text
			else:
				return None

	def _set(self):
		self.resource_uri = self.xmlresp.nsmap[self.xmlresp.prefix]
		self.ElementName = self.get_value("ElementName")
		self.DeviceID  = self.get_value("DeviceID")
		self.CreationClassName  = self.get_value("CreationClassName")

	def Enumerate(self):
		items = Wsman.Enumerate(self, self.resource_uri)
		instances = []
		for batch in items:
			for item in batch:
				cs = CIM_Processor(self.cl)
				cs.xmlresp = item
				cs._set()
				instances.append(cs)
		return instances


	def Get(self, selectors):
		resp = Wsman.Get(self, self.resource_uri, selectors)
		env = ElementTree.parse(StringIO(resp))
		self.xmlresp = env.xpath("//s:Body/*", self.namespaces)[0]
		self._set()




class CIM_ComputerSystem(Wsman):
	resource_uri =  "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"
	classname =  "CIM_ComputerSystem"
	def __init__(self, arg):
		Wsman.__init__(self, arg)
		self.cl = arg
		self.Dedicated = None
		self.CreationClassName = None
		self.Name = None
		self.xmlresp = None

	def dump(self):
		if self.Dedicated:
			print "Dedicated: " + self.Dedicated
		if self.CreationClassName:
			print "CreationClassName: " + self.CreationClassName
		if self.Name:
			print "Name: " + self.Name

	def get_path(self):
		return "%s?CreationClassName=%s&Name=%s" %(resource_uri, self.CreationClassName, self.Name)

	def get_value(self, prop):
		element = None
		for el in self.xmlresp.getiterator():
			res = self.xmlresp.xpath(el.prefix + ":" + prop , el.nsmap )
			if isinstance(res, list) and len(res) > 0 :
				return res[0].text
			else:
				return None

	def _set(self):
		self.resource_uri = self.xmlresp.nsmap[self.xmlresp.prefix]
		self.Dedicated  = self.get_value("Dedicated")
		self.Name  = self.get_value("Name")
		self.CreationClassName  = self.get_value("CreationClassName")

	def Enumerate(self):
		items = Wsman.Enumerate(self, self.resource_uri)
		instances = []
		for batch in items:
			for item in batch:
				cs = CIM_ComputerSystem(self.cl)
				cs.xmlresp = item
				cs._set()
				instances.append(cs)
		return instances



	def Get(self, selectors):
		resp = Wsman.Get(self, self.resource_uri, selectors)
		env = ElementTree.parse(StringIO(resp))
		self.xmlresp = env.xpath("//s:Body/*", self.namespaces)[0]
		self._set()

	#def Associators(self, object):


class CIM_Capabilities(Wsman):
	resource_uri =  "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_Capabilities"
	classname =  "CIM_Capabilities"
	def __init__(self, arg):
		Wsman.__init__(self, arg)
		self.cl = arg
		self.InstanceID = None
		self.ElementName = None

		self.xmlresp = None

	#def get_path(self):
	#	return "%s?CreationClassName=%s&Name=%s" %(resource_uri, self.CreationClassName, self.Name)

	def dump(self):
		if self.InstanceID:
			print "InstanceID: " + self.InstanceID
		if self.ElementName:
			print "ElementName: " + self.ElementName

	def get_value(self, prop):
		for el in self.xmlresp.getiterator():
			res = self.xmlresp.xpath(el.prefix + ":" + prop , el.nsmap )
			if isinstance(res, list):
				return res[0].text
			else:
				return None

	def _set(self):
		self.resource_uri = self.xmlresp.nsmap[self.xmlresp.prefix]
		self.InstanceID  = self.get_value("InstanceID")
		self.ElementName  = self.get_value("ElementName")

	def Enumerate(self):
		items = Wsman.Enumerate(self, self.resource_uri)
		instances = []
		for batch in items:
			for item in batch:
				cs = CIM_Capabilities(self.cl)
				cs.xmlresp = item
				cs._set()
				instances.append(cs)
		return instances



	def Get(self, selectors):
		resp = Wsman.Get(self, self.resource_uri, selectors)
		env = ElementTree.parse(StringIO(resp))
		self.xmlresp = env.xpath("//s:Body/*", self.namespaces)[0]
		self._set()



class CIM_ManagedElement(Wsman):
	resource_uri =  "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ManagedElement"
	classname =  "CIM_ManagedElement"
	def __init__(self, arg):
		Wsman.__init__(self, arg)
		self.cl = arg
		self.Caption = None
		self.Description = None
		self.ElementName = None

		self.xmlresp = None

	#def get_path(self):
	#	return "%s?CreationClassName=%s&Name=%s" %(resource_uri, self.CreationClassName, self.Name)

	def dump(self):
		if self.Caption:
			print "Caption: " + self.Caption
		if self.Description:
			print "Description: " + self.Description
		if self.ElementName:
			print "ElementName: " + self.ElementName

	def get_value(self, prop):
		element = None
		for el in self.xmlresp.getiterator():
			res = self.xmlresp.xpath(el.prefix + ":" + prop , el.nsmap )
			if isinstance(res, list) and len(res) > 0 :
				return res[0].text
			else:
				return None

	def _set(self):
		self.resource_uri = self.xmlresp.nsmap[self.xmlresp.prefix]
		self.Caption  = self.get_value("Caption")
		self.Description  = self.get_value("Description")
		self.ElementName  = self.get_value("ElementName")

	def Enumerate(self):
		items = Wsman.Enumerate(self, self.resource_uri)
		instances = []
		for batch in items:
			for item in batch:
				cs = CIM_ManagedElement(self.cl)
				cs.xmlresp = item
				cs._set()
				instances.append(cs)
		return instances



	def Get(self, selectors):
		resp = Wsman.Get(self, self.resource_uri, selectors)
		env = ElementTree.parse(StringIO(resp))
		self.xmlresp = env.xpath("//s:Body/*", self.namespaces)[0]
		self._set()


	#def Associators(self, object):

class CIM_ElementCapabilities(Wsman):
	resource_uri =  "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ElementCapabilities"
	classname =  "CIM_ElementCapabilities"
	def __init__(self, arg):
		Wsman.__init__(self, arg)
		self.cl = arg

		self.ManagedElement = None
		self.Capabilities = None

		self.xmlresp = None
	def get_value(self, prop):
		for el in self.xmlresp.getiterator():
			res = self.xmlresp.xpath(el.prefix + ":" + prop , el.nsmap )
			if isinstance(res, list):
				return res[0].text
			else:
				return None
	def _set(self):
		self.resource_uri = self.xmlresp.nsmap[self.xmlresp.prefix]
		self.ManagedElement = CIM_ComputerSystem(self.cl)
		epr = self.get_epr(self.xmlresp, "ManagedElement");
		if epr:
			self.ManagedElement.get_from_epr(epr)

		self.Capabilities  = CIM_Capabilities(self.cl)
		epr = self.get_epr(self.xmlresp, "Capabilities");
		if epr:
			self.Capabilities.get_from_epr(epr)

	def Enumerate(self):
		items = Wsman.Enumerate(self, self.resource_uri)
		instances = []
		for batch in items:
			for item in batch:
				cs = CIM_ElementCapabilities(self.cl)
				cs.xmlresp = item
				cs._set()
				instances.append(cs)
		return instances


	def Get(self, selectors):
		resp = Wsman.Get(self, self.resource_uri, selectors)
		env = ElementTree.parse(StringIO(resp))
		self.xmlresp = env.xpath("//s:Body/*", self.namespaces)[0]
		self._set()





print
print "Get Test 1"
print "#################################"

cl = WsmanClient()
cl.create("localhost", 16992, "/wsman", "http", "admin", "Admin@98")

cs = CIM_ComputerSystem(cl)

#selectors = {'CreationClassName':'Linux_ComputerSystem','Name':'bl004.home.planux.com'}
selectors = {'__GUID': '03000000-0000-0000-0000-000000000000'}
cs.Get(selectors)

print cs.Dedicated
print cs.CreationClassName
print cs.Name

print
print "Enumerate Test 1"
print "#################################"

cs = CIM_ComputerSystem(cl)
lcs = cs.Enumerate()
print ("instances: %d" %(len(lcs) ) )
for inst in lcs:
	print "Instance: " + inst.resource_uri   + " { "
	print "  Dedicated: " + inst.Dedicated
	print "  Name: " + inst.Name
	print "  CreationClassName: " + inst.CreationClassName
	print "}"
	print



print
print "Enumerate Test 2"
print "#################################"

proc = CIM_Processor(cl)
lproc = proc.Enumerate()
print ("instances: %d" %(len(lproc) ) )
for inst in lproc:
	print "Instance: " + inst.resource_uri   + " { "
	print "  ElementName: " + inst.ElementName
	print "  DeviceID: " + inst.DeviceID
	print "  CreationClassName: " + inst.CreationClassName
	print "}"
	print

print
print "Enumerate Test 3"
print "#################################"

cls = CIM_ElementCapabilities(cl)
lcls = cls.Enumerate()
print ("instances: %d" %(len(lcls) ) )
for inst in lcls:
	print "Instance: " + inst.resource_uri   + " { "
	print "ManagedElement " +  inst.ManagedElement.resource_uri + " {"
	inst.ManagedElement.dump()
	print "}"
	print "Capabilities " + inst.Capabilities.resource_uri + " {"
	inst.Capabilities.dump()
	print "}"
	print "}"
	print

