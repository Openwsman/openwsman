import pywsman as OpenWSMan
from lxml import etree as ElementTree
from lxml import objectify
from StringIO import StringIO

NS_SOAP_12 = "http://www.w3.org/2003/05/soap-envelope"
NS_WS_ADDRESSING = "http://schemas.xmlsoap.org/ws/2004/08/addressing"
NS_WS_MANAGEMENT = "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
NS_WS_ENUMERATION = "http://schemas.xmlsoap.org/ws/2004/09/enumeration"
NS_WS_TRANSFER = "http://schemas.xmlsoap.org/ws/2004/09/transfer"
NS_WS_EVENTNG = "http://schemas.xmlsoap.org/ws/2004/08/eventing"
FABRIKAM = "http://fabrikam.org/fabrikam"

WSM_ASSOCIATION_FILTER_DIALECT  =  "http://schemas.dmtf.org/wbem/wsman/1/cimbinding/associationFilter"
ALL_CLASS_URI =  "http://schemas.dmtf.org/wbem/wscim/1/*"

namespaces =  { "s": NS_SOAP_12 ,"wsa": NS_WS_ADDRESSING , "wsen": NS_WS_ENUMERATION,
		"wxf": NS_WS_TRANSFER,"wsman": NS_WS_MANAGEMENT , "wse": NS_WS_EVENTNG, 
		"fabrikam": FABRIKAM}



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

	def toxml(self, rootElement , rootNs):
		root = ElementTree.Element("{" + rootNs + "}" + rootElement)
		address = ElementTree.SubElement(root, "{" + NS_WS_ADDRESSING + "}Address")
		address.text = self._Address
		param = ElementTree.SubElement(root, "{" + NS_WS_ADDRESSING + "}ReferenceParameters")
		ruri = ElementTree.SubElement(param, "{" + NS_WS_MANAGEMENT + "}ResourceURI")
		ruri.text = self._ReferenceParameters._resource_uri
		sset = ElementTree.SubElement(param, "{" + NS_WS_MANAGEMENT + "}SelectorSet")
		selectors = self._ReferenceParameters._selector_set
		for selector in selectors.keys():
			s = ElementTree.SubElement(sset, "{" + NS_WS_MANAGEMENT + "}Selector")
			s.set("Name", selector)
			s.text = selectors[selector]

		return root


class ReferenceParameters:
	def setResourceURI(self, value):
		self._resource_uri = value

    	def setSelectorSet(self, value):
	    	self._selector_set = value



class WsmanClient:
	debug = 0
	def __init__(self, hostname, port , path, scheme, username, password):
		self.uri = None
		self.hostname = hostname
		self.port = port
		self.path = path
		self.scheme = scheme
		self.username = username
		self.password = password
		self.hdl = OpenWSMan.wsmc_create(hostname, port , path, scheme, username, password)

	def create_from_uri(self, uri):
		self.hdl = OpenWSMan.wsmc_create_from_uri(uri)
	def release(self):
		OpenWSMan.wsmc_release(self.hdl)

class WsmanException(Exception):
	def __init__(self, value):
		self.value = value
	def __str__(self):
		return repr(self.value)

class Wsman:
	debug = 0
	namespaces =  { "s": NS_SOAP_12 ,"wsa": NS_WS_ADDRESSING , "wsen": NS_WS_ENUMERATION,
		"wxf": NS_WS_TRANSFER,"wsman": NS_WS_MANAGEMENT , "wse": NS_WS_EVENTNG, 
		"fabrikam": FABRIKAM}
	encoding = 'UTF-8';
	def __init__(self, arg):
		self.cl = arg
		self.debug = self.cl.debug

	def get_from_epr(self, epr):
		resp = Wsman.Get(self, epr._ReferenceParameters._resource_uri, epr._ReferenceParameters._selector_set)
		env = ElementTree.parse(StringIO(resp))
		self.xmlresp = env.xpath("//s:Body/*", self.namespaces)[0]
		self._set()

	def get_array_value(self, prop):
		arr = []
	    	for el in self.xmlresp.getiterator():
		    	res = self.xmlresp.xpath(el.prefix + ":" + prop , el.nsmap )
		    	if isinstance(res, list) and len(res) > 0 :
			    	for i in res:
				    	arr.append(i.text)
			    	return arr
	    	return arr

	def get_value(self, prop):
	    	for el in self.xmlresp.getiterator():
		    	res = self.xmlresp.xpath(el.prefix + ":" + prop , el.nsmap )
		    	if isinstance(res, list) and len(res) > 0 :
			    	return res[0].text
		    	else:
			    	return None

	def _getObject(self, classname, arg):
	    	exec "from %s import %s" %(classname, classname)
	    	exec "c = %s(self.cl)" %(classname)
	    	return c

	def _getClassName(self, epr):
	    	ruri = epr._ReferenceParameters._resource_uri
	    	clsname_split = ruri.split('/')
	    	clsname = clsname_split[len(clsname_split)-1]
	    	return clsname

	def dump(self, indent=""):
	    	print indent + "Instance of " + self.resource_uri + " {"
	    	self.dump_properties(indent)
	    	print indent + "}"



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


	def get_textual_value(self, key, val, isKey=False, array=False, valueMap={}, indent=""):
		if not val:
			if array:
				return indent + "  " + key + ": []"
			else:
				return indent + "  " + key + ": "

		if array:
			arr = ""
			for b in val:
				if valueMap.has_key(b):
					arr =  arr + valueMap[b] + ", "
				else:
					arr = arr + b + ",  "
			return indent + "  " + key + ": [  " + arr + "]"
		else:
			if valueMap.has_key(val):
				st =  valueMap[val]
				return indent + "  " + key + ": " + st
			else:
				if isKey:
					return indent + "  [" + key + "]: " + val
				else:
					return indent + "  " + key + ": " + val
	def GetContext(self, xmlresp):
		context = xmlresp.xpath("/s:Envelope/s:Body/wsen:EnumerateResponse/wsen:EnumerationContext", self.namespaces )
		return context
	def GetPullContext(self, xmlresp):
		context = xmlresp.xpath("/s:Envelope/s:Body/wsen:PullResponse/wsen:EnumerationContext", self.namespaces )
		return context
	def GetItems(self, xmlresp):
		items = xmlresp.xpath("/s:Envelope/s:Body/wsen:PullResponse/wsen:Items/*", self.namespaces )
		return items

	def WsmanFault(self,resp,  e):
		env = ElementTree.parse(StringIO(resp))
		subcode = env.xpath("//s:Body/s:Fault/s:Code/s:Subcode/s:Value", self.namespaces)[0]
		reason = env.xpath("//s:Body/s:Fault/s:Reason/s:Text", self.namespaces)[0]
		if subcode.text:
			print "Wsman Exception: %s" % subcode.text
		if reason.text:
			print "Reason: %s" % reason.text
		print
		exit()

	def toxml(self):
		root = ElementTree.Element("{" +  self.resource_uri + "}" + self.classname )
		p = self._getProperties()
		for k in p.keys():
			h = p[k]
			if h['ContentType'] == 'cim:cimReference' and h['Value'] is not None:
				epr = h['Value'].Epr()
				root.append(epr.toxml(k, self.resource_uri))
			elif  h['Value'] is not None and h['isArray']:
				for item in h['Value']:
					_xml = ElementTree.SubElement(root, "{" +  self.resource_uri + "}" + k)
					_xml.text = str(item)
			elif h['Value'] is not None:
				_xml = ElementTree.SubElement(root, "{" +  self.resource_uri + "}" + k)
				if h['Type'] == 'string':
					_xml.text = str(h['Value'])
				elif h['Type'] == 'boolean' and h['Value']:
					_xml.text = "true"
				elif h['Type'] == 'boolean' and not h['Value']:
					_xml.text = "false"
				else:
					_xml.text = str(h['Value'])

		return root



	def Epr(self):
		epr = EndPointReference()
		param = ReferenceParameters()
		epr.setAddress( "%s://%s:%d/wsman" %(self.cl.scheme, self.cl.hostname, self.cl.port ) )
		param.setResourceURI(self.resource_uri)
		param.setSelectorSet(self.get_selectors())
		epr.setReferenceParameters(param)
		return epr


	def Identify(self):
		op = OpenWSMan.wsmc_options_init()
		if self.debug == 1:
			OpenWSMan.wsmc_set_action_option(op, OpenWSMan.FLAG_DUMP_REQUEST)
		resp = OpenWSMan._identify(self.cl.hdl, op , self.encoding)
		xmlresp = ElementTree.parse(StringIO(resp))
		return xmlresp
	def Subscribe(self, ruri, **param):
		op = OpenWSMan.wsmc_options_init()
		if self.debug == 1:
			print "Request: > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > >"
			OpenWSMan.wsmc_set_action_option(op, OpenWSMan.FLAG_DUMP_REQUEST)
		for x in param.keys():
			if x == "filter":
				OpenWSMan.wsmc_set_filter(param[x], op)
			elif x == "dialect":
				OpenWSMan.wsmc_set_dialect(param[x], op)
			elif x == "notification_uri" and param[x]:
				OpenWSMan.wsmc_set_delivery_uri(param[x], op)
			elif x == "expiry_time" and param[x]:
				OpenWSMan.wsmc_set_sub_expiry(param[x], op)	
			elif x == "heartbeat" and param[x]:
				OpenWSMan.wsmc_set_heartbeat_interval(param[x], op)
			elif x == "mode" and param[x]:
				if param[x] == 'push':
					OpenWSMan.wsmc_set_delivery_mode(OpenWSMan.WSMAN_DELIVERY_PUSH, op)
				elif param[x] == 'pushwithack':								
					OpenWSMan.wsmc_set_delivery_mode(OpenWSMan.WSMAN_DELIVERY_PUSHWITHACK, op)
				elif param[x] == 'events':
					OpenWSMan.wsmc_set_delivery_mode(OpenWSMan.WSMAN_DELIVERY_EVENTS, op)
				elif param[x] == 'pull':
					OpenWSMan.wsmc_set_delivery_mode(OpenWSMan.WSMAN_DELIVERY_PULL, op)	
		resp = None;
		try:											
			resp = OpenWSMan._subscribe(self.cl.hdl,  ruri, op,  self.encoding)
			code = OpenWSMan.wsmc_get_response_code(self.cl.hdl)
			if (code != 200): raise WsmanException(code)
		except  WsmanException, e:
			self.WsmanFault(resp, e)
		else:			
			xmlresp = ElementTree.parse(StringIO(resp))
			if self.debug:
				print "< < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < <"			
				print ElementTree.tostring(xmlresp, pretty_print=True)
		
			identifier = xmlresp.xpath("/s:Envelope/s:Body/wse:SubscribeResponse/wse:SubscriptionManager/wsa:ReferenceParameters/wse:Identifier", self.namespaces )			
			if identifier != None:
				return identifier[0].text
			else:
				return None
		

	def Enumerate(self, ruri, **param):
		op = OpenWSMan.wsmc_options_init()
		if self.debug == 1:
			print "Request: > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > >"
			OpenWSMan.wsmc_set_action_option(op, OpenWSMan.FLAG_DUMP_REQUEST)
		for x in param.keys():
			if x == "filter":
				OpenWSMan.wsmc_set_filter(param[x], op)
			elif x == "dialect":
				OpenWSMan.wsmc_set_dialect(param[x], op)
			elif x == "assoc" and param[x]:
				OpenWSMan._set_assoc_filter()
		items = []
		resp = OpenWSMan._enumerate(self.cl.hdl,  ruri, op,  self.encoding)
		if self.debug:
			print "< < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < <"
			env = ElementTree.parse(StringIO(resp))
			print ElementTree.tostring(env, pretty_print=True)
		xmlresp = ElementTree.parse(StringIO(resp))
		context = self.GetContext(xmlresp)
		while context:
			resp = OpenWSMan._pull(self.cl.hdl,  ruri, op, context[0].text , self.encoding)
			xmlresp = ElementTree.parse(StringIO(resp))
			if self.debug:
				print "< < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < <"				
				print ElementTree.tostring(xmlresp, pretty_print=True)			
			items.append(self.GetItems(xmlresp))
			context = self.GetPullContext(xmlresp)

		return items

	def Invoke(self, ruri, method, selectors, input):
		resp = None
		op = OpenWSMan.wsmc_options_init()
		if self.debug == 1:
			print "Request: > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > >"
			OpenWSMan.wsmc_set_action_option(op, OpenWSMan.FLAG_DUMP_REQUEST)
		for x in selectors.keys():
			OpenWSMan.wsmc_add_selector(op, x, selectors[x])


		try:
			resp = OpenWSMan._invoke(self.cl.hdl, ruri, op, method, input, len(input), self.encoding)
			if self.debug:
				print "< < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < <"
				env = ElementTree.parse(StringIO(resp))
				print ElementTree.tostring(env, pretty_print=True)

			code = OpenWSMan.wsmc_get_response_code(self.cl.hdl)
			if (code != 200): raise WsmanException(code)
		except WsmanException, e:
			self.WsmanFault(resp, e)
		return resp

	def Put(self, input , ResourceURI=None, Selectors={}):
		resp = None
		op = OpenWSMan.wsmc_options_init()
		if self.debug == 1:
			print "Request: > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > >"
			OpenWSMan.wsmc_set_action_option(op, OpenWSMan.FLAG_DUMP_REQUEST)
		for sel in Selectors.keys():
			OpenWSMan.wsmc_add_selector(op, sel, Selectors[sel])
		if ResourceURI is None:
			ResourceURI = self.resource_uri
		try:
			resp = OpenWSMan._put(self.cl.hdl, ResourceURI, op, input, len(input), self.encoding)
			if self.debug:
				print "< < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < <"
				env = ElementTree.parse(StringIO(resp))
				print ElementTree.tostring(env, pretty_print=True)
			code = OpenWSMan.wsmc_get_response_code(self.cl.hdl)
			if (code != 200): raise WsmanException(code)
		except WsmanException, e:
			self.WsmanFault(resp, e)

		return resp


	# FIXME
	def Associations(self, object):
		path = obgect.getpath()
		items = Wsman.Enumerate(self, allclass, dialect=WSM_ASSOCIATION_FILTER_DIALECT, assoc=True , filter=path)
		return items

	def Get(self, ResourceURI=None, Selectors={}):
		op = OpenWSMan.wsmc_options_init()
		if self.debug == 1:
			print "Request: > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > > >"
			OpenWSMan.wsmc_set_action_option(op, OpenWSMan.FLAG_DUMP_REQUEST)

		#if Selectors == {}:
		#	Selectors = self.get_selectors()

		for selector in Selectors.keys():
			OpenWSMan.wsmc_add_selector(op, selector, Selectors[selector])
		if ResourceURI is None:
			ResourceURI = self.resource_uri


		try:
			resp = OpenWSMan._get(self.cl.hdl, ResourceURI, op, self.encoding)
			code = OpenWSMan.wsmc_get_response_code(self.cl.hdl)
			if (code != 200): raise WsmanException(code)
			if self.debug:
				print "< < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < < <: Response"
				env = ElementTree.parse(StringIO(resp))
				print ElementTree.tostring(env, pretty_print=True)
		except WsmanException, e:
			self.WsmanFault(resp, e)

		return resp





