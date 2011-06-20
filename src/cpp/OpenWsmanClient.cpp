//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2007.
//
//  File:       OpenWsmanClient.cpp
//
//  Contents:   An implementation of the WsmanClient interface using openwsman
//
//----------------------------------------------------------------------------

#include "OpenWsmanClient.h"


extern "C" {
#include "u/libu.h"
#include "wsman-api.h"
}

#include "wsman-client-transport.h"
#include "wsman-filter.h"

#define WSMAN_ENCODING		"UTF-8"

using namespace WsmanClientNamespace;

static bool CheckWsmanResponse(WsManClient* cl, WsXmlDocH& doc);
static bool ResourceNotFound(WsManClient* cl, WsXmlDocH& enumerationRes);
static string XmlDocToString(WsXmlDocH& doc);
static client_opt_t *SetOptions(WsManClient* cl);
static string GetSubscribeContext(WsXmlDocH& doc);
static string ExtractPayload(WsXmlDocH& doc);
static string ExtractItems(WsXmlDocH& doc);

// Construct from params.

OpenWsmanClient::OpenWsmanClient(const char *host,
		const int port,
		const char *path ,
		const char *scheme,
		const char *auth_method ,
		const char *username,
		const char *password,
		 // proxy address include proxy port
		const char *proxy,
		//proxy user name 
		const char *proxy_username,
		//proxy password
		const char *proxy_password
#ifdef _WIN32
		// determines which cert store to search
		,const bool local,
		// search for a client cert with this name
		const char *cert,
		// search for a cient cert with this oid
		const char *oid
#endif
		)
{
	cl = wsmc_create(host, port, path, scheme, username, password);
	SetAuth(auth_method);	
#ifdef _WIN32
	SetClientCert(oid, cert, local);
#endif
	SetProxy(proxy,proxy_username,proxy_password);
	wsmc_transport_init(cl, (void*)NULL);
}

// Destructor.
OpenWsmanClient::~OpenWsmanClient() 
{
	wsmc_transport_fini(cl);
	wsmc_release(cl);
}

string OpenWsmanClient::Identify() const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	WsXmlDocH identifyResponse = wsmc_action_identify(cl, 		
			options
			);
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, identifyResponse);
	string xml = ExtractPayload(identifyResponse);
	ws_xml_destroy_doc(identifyResponse);
	return xml; 
}

string OpenWsmanClient::Create(const string &resourceUri, const string &data) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	WsXmlDocH createResponse = wsmc_action_create_fromtext(cl, 
			resourceUri.c_str(),
			options,
			data.c_str(), data.length(), WSMAN_ENCODING);
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, createResponse);
	string xml = ExtractPayload(createResponse);
	ws_xml_destroy_doc(createResponse);
	return xml; 
}

void OpenWsmanClient::Delete(const string &resourceUri, const NameValuePairs *s) const
{
	client_opt_t *options;
	options = SetOptions(cl);
	if(s)
	{
		// Add selectors.
		for (PairsIterator p = s->begin(); p != s->end(); ++p) {
			if(p->second != "")
				wsmc_add_selector(options, 
						(char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}
	WsXmlDocH deleteResponse = wsmc_action_delete(	cl, 
			(char *)resourceUri.c_str(),
			options);
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, deleteResponse);
	ws_xml_destroy_doc(deleteResponse);
}

void OpenWsmanClient::Enumerate(const string &resourceUri, vector<string> &enumRes, const NameValuePairs *s) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	if(s)
	{
		// Add selectors.
		for (PairsIterator p = s->begin(); p != s->end(); ++p) {
			if(p->second != "")
				wsmc_add_selector(options, 
						(char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}

	WsXmlDocH doc;
	char *enumContext;
	WsXmlDocH enum_response = wsmc_action_enumerate(cl, (char *)resourceUri.c_str(),  options, NULL);

	try
	{
		if(ResourceNotFound(cl, enum_response))
		{
			wsmc_options_destroy(options);
			return;
		}
	}
	catch(WsmanSoapFault& e)
	{
		wsmc_options_destroy(options);
		throw e;
	}
	catch(WsmanClientException& e)
	{
		wsmc_options_destroy(options);
		throw e;
	}
	catch(exception& e)
	{
		wsmc_options_destroy(options);
		throw e;
	}

	enumContext = wsmc_get_enum_context(enum_response);
	ws_xml_destroy_doc(enum_response);

	while (enumContext != NULL && enumContext[0] != 0 ) {
		doc = wsmc_action_pull(cl, resourceUri.c_str(), options, NULL, enumContext);
		try
		{
			CheckWsmanResponse(cl, doc);
		}
		catch(exception& e)
		{
			wsmc_options_destroy(options);
			throw e;
		}
		string payload = ExtractItems(doc);
		if (payload.length() > 0)
			enumRes.push_back(payload);
		u_free(enumContext);
		enumContext = wsmc_get_enum_context(doc);    
		ws_xml_destroy_doc(doc);
	}
	u_free(enumContext);
	wsmc_options_destroy(options);
}

void OpenWsmanClient::Enumerate(const string & resourceUri, WsmanFilter & filter, vector<string> &enumRes) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	
	WsXmlDocH doc;
	char *enumContext;
	WsXmlDocH enum_response = wsmc_action_enumerate(cl, (char *)resourceUri.c_str(),  options, filter.getfilter());

	try
	{
		if(ResourceNotFound(cl, enum_response))
		{
			wsmc_options_destroy(options);
			return;
		}
	}
	catch(WsmanSoapFault& e)
	{
		wsmc_options_destroy(options);
		throw e;
	}
	catch(WsmanClientException& e)
	{
		wsmc_options_destroy(options);
		throw e;
	}
	catch(exception& e)
	{
		wsmc_options_destroy(options);
		throw e;
	}

	enumContext = wsmc_get_enum_context(enum_response);
	ws_xml_destroy_doc(enum_response);

	while (enumContext != NULL && enumContext[0] != 0 ) {
		doc = wsmc_action_pull(cl, resourceUri.c_str(), options, NULL, enumContext);
		try
		{
			CheckWsmanResponse(cl, doc);
		}
		catch(exception& e)
		{
			wsmc_options_destroy(options);
			throw e;
		}
		string payload = ExtractItems(doc);
		if (payload.length() > 0)
			enumRes.push_back(payload);
		u_free(enumContext);
		enumContext = wsmc_get_enum_context(doc);    
		ws_xml_destroy_doc(doc);
	}
	u_free(enumContext);
	wsmc_options_destroy(options);
}

string OpenWsmanClient::Get(const string &resourceUri, const NameValuePairs *s) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	WsXmlDocH doc;
	// Add selectors.
	if (s) {
		for (PairsIterator p = s->begin(); p != s->end(); ++p) {
			if(p->second != "")
				wsmc_add_selector(options, 
						(char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}
	doc = wsmc_action_get(cl, (char *)resourceUri.c_str(), options);
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Put(const string &resourceUri, const string &content, const NameValuePairs *s) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	WsXmlDocH doc;
	// Add selectors.
	if (s) {
		for (PairsIterator p = s->begin(); p != s->end(); ++p) {
			if(p->second != "")
				wsmc_add_selector(options, 
						(char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}
	doc = wsmc_action_put_fromtext(cl, resourceUri.c_str(), options, content.c_str(), content.length(), WSMAN_ENCODING);
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Invoke(const string &resourceUri, const string &methodName, const string &content, const NameValuePairs *s) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	WsXmlDocH doc;
	string error;

	// Add selectors.
	if (s) {
		for (PairsIterator p = s->begin(); p != s->end(); ++p) {
			if(p->second != "")
				wsmc_add_selector(options, 
						(char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}
	doc = wsmc_action_invoke_fromtext(cl, resourceUri.c_str(), options,
			(char *)methodName.c_str(), content.c_str(),
			content.length(), WSMAN_ENCODING);
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Subscribe(const string &resourceUri, const SubscribeInfo &info, string &subsContext) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	WsXmlDocH doc;
	options->delivery_mode = (WsmanDeliveryMode)info.delivery_mode;
	options->delivery_uri = u_strdup(info.delivery_uri.c_str());
	if(info.dialect !=  "" && info.filter != "") {		
		filter_create_simple(info.dialect.c_str(), info.filter.c_str());
	}
	
	if(info.refenceParam != "")
		options->reference = u_strdup(info.refenceParam.c_str());
		// Add selectors.
	if (info.selectorset) {
		for (PairsIterator p = info.selectorset->begin(); p != info.selectorset->end(); ++p) {
			if(p->second != "")
				wsmc_add_selector(options, 
						(char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}
	options->expires = info.expires;
	options->heartbeat_interval = info.heartbeat_interval;
	doc = wsmc_action_subscribe(cl, (char *)resourceUri.c_str(), options, NULL);
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	subsContext = GetSubscribeContext(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Renew(const string &resourceUri, const string &subsContext, float expire, const NameValuePairs *s) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	WsXmlDocH doc;
	options->expires = expire;
	if (s) {
		for (PairsIterator p = s->begin(); p != s->end(); ++p) {
			if(p->second != "")
				wsmc_add_selector(options, 
						(char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}
	doc = wsmc_action_renew(cl, (char *)resourceUri.c_str(), options, subsContext.c_str());
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}
			
void OpenWsmanClient::Unsubscribe(const string &resourceUri, const string &subsContext, const NameValuePairs *s) const
{
	client_opt_t *options = NULL;
	options = SetOptions(cl);
	WsXmlDocH doc;
	if (s) {
		for (PairsIterator p = s->begin(); p != s->end(); ++p) {
			if(p->second != "")
				wsmc_add_selector(options, 
						(char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}
	doc = wsmc_action_unsubscribe(cl, (char *)resourceUri.c_str(), options, subsContext.c_str());
	wsmc_options_destroy(options);
	CheckWsmanResponse(cl, doc);
	ws_xml_destroy_doc(doc);
	return;
}


string GetSubscribeContext(WsXmlDocH& doc)
{
	string str;
	char *buf = NULL;
	WsXmlNodeH bodyNode = ws_xml_get_soap_body(doc);
	WsXmlNodeH tmp = NULL;
	if(bodyNode == NULL) return str;
	bodyNode = ws_xml_get_child(bodyNode, 0, XML_NS_EVENTING, WSEVENT_SUBSCRIBE_RESP);
	if(bodyNode == NULL) return str;
	bodyNode = ws_xml_get_child(bodyNode, 0, XML_NS_EVENTING, WSEVENT_SUBSCRIPTION_MANAGER);
	if(bodyNode == NULL) return str;
	tmp = ws_xml_get_child(bodyNode, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);
	if(tmp == NULL) {
		tmp = ws_xml_get_child(bodyNode, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PROPERTIES);
		if(tmp == NULL) return str;
	}
	wsmc_node_to_buf(tmp, &buf);
	str = string(buf);
	u_free(buf);
	return str;
}

string ExtractPayload(WsXmlDocH& doc)
{
	WsXmlNodeH bodyNode = ws_xml_get_soap_body(doc);	  
	WsXmlNodeH payloadNode = ws_xml_get_child(bodyNode, 0, NULL, NULL);
	char *buf = NULL;
	wsmc_node_to_buf( payloadNode, &buf);
	string payload = string(buf);
	u_free(buf);
	return payload;
}

string ExtractItems(WsXmlDocH& doc)
{
	string payload;
	WsXmlNodeH bodyNode = ws_xml_get_soap_body(doc);
	WsXmlNodeH pullResponse = ws_xml_get_child(bodyNode, 0, XML_NS_ENUMERATION, WSENUM_PULL_RESP);
	WsXmlNodeH itemsNode = ws_xml_get_child(pullResponse, 0, XML_NS_ENUMERATION, WSENUM_ITEMS);
	WsXmlNodeH n = ws_xml_get_child(itemsNode, 0 , NULL, NULL );
	if (n) {
		char *buf = NULL;
		wsmc_node_to_buf( n, &buf);
		payload = string(buf);
		u_free(buf);
		
	}
	return payload;
}

string XmlDocToString(WsXmlDocH& doc) {
	char *buf;
	int	  len;
	ws_xml_dump_memory_enc(doc, &buf, &len, WSMAN_ENCODING);
	string str = string(buf);	// This constructor copies the data.
	if (buf)
#ifdef _WIN32
		ws_xml_free_memory(buf);
#else
	u_free(buf);
#endif
	return str;
}

client_opt_t * SetOptions(WsManClient* cl)
{
	client_opt_t *options = wsmc_options_init();
	char *ns = wsmc_get_namespace(cl);
	if(ns)
		options->cim_ns = u_strdup(ns);
	return options;
}

bool CheckWsmanResponse(WsManClient* cl, WsXmlDocH& doc)
{
	long lastError = wsmc_get_last_error(cl);
	string error;
	if(lastError)
	{
		char tmp[10];
		error = "Failed to establish a connection with the server.\n";
		sprintf(tmp, "%ld", lastError);
		error.append("Openwsman last error = ").append(tmp);
		ws_xml_destroy_doc(doc);
		throw WsmanClientException(error.c_str(), WSMAN_CONNECT_ERROR);
	}
	long responseCode = wsmc_get_response_code(cl);
	if (responseCode != 200 &&
			responseCode != 400 &&
			responseCode != 500)
	{
		char tmp[10];
		error = "An HTTP error occurred.\n";
		sprintf(tmp, "%ld", responseCode);
		error.append("HTTP Error = ").append(tmp);
		ws_xml_destroy_doc(doc);
		throw WsmanClientException(error.c_str(), WSMAN_HTTP_ERROR);
	}
	if(!doc)
	{
		throw WsmanClientException("The Wsman response was NULL.");
	}
	if (wsmc_check_for_fault(doc)) {
		char tmp[10];
		WsManFault *fault = wsmc_fault_new();
		wsmc_get_fault_data(doc, fault);
		string subcode_s = fault->subcode ? string(fault->subcode) : "";	
		string code_s = fault->code ? string(fault->code) : "";
		string reason_s = fault->reason ? string(fault->reason) : "";
		string detail_s = fault->fault_detail ? string(fault->fault_detail) : "";
		ws_xml_destroy_doc(doc);
		wsmc_fault_destroy(fault);
		error = "A Soap Fault was received:";
		error.append("\nFaultCode: " + code_s);
		error.append("\nFaultSubCode: " + subcode_s);
		error.append("\nFaultReason: " + reason_s);
		error.append("\nFaultDetail: " + detail_s);
		sprintf(tmp, "%ld", responseCode);
		error.append("\nHttpCode:  = ").append(tmp);
		throw WsmanSoapFault(error.c_str(), code_s, subcode_s, reason_s, detail_s);
	}
	return true;
}

bool ResourceNotFound(WsManClient* cl, WsXmlDocH& enumerationRes)
{
	long responseCode = wsmc_get_response_code(cl);
	if(wsmc_get_last_error(cl) ||
			(responseCode != 200 && responseCode != 400 && responseCode != 500) ||
			!enumerationRes)
	{
		CheckWsmanResponse(cl, enumerationRes);
	}
	if (!wsmc_check_for_fault(enumerationRes))
	{
		return false;
	}
	WsManFault *fault = wsmc_fault_new();
	bool ret = false;
	wsmc_get_fault_data(enumerationRes, fault);
	string subcode_s = fault->subcode ? string(fault->subcode) : "";
	if(subcode_s.find("DestinationUnreachable") != string::npos)
	{
		wsmc_fault_destroy(fault);
		return true;
	}
	wsmc_fault_destroy(fault);
	if(!ret)
	{
		CheckWsmanResponse(cl, enumerationRes);
	}
	return ret;
}

void OpenWsmanClient::SetAuth(const char *auth_method)
{
	wsman_transport_set_auth_method (cl , (char *)auth_method);
	if (wsmc_transport_get_auth_value(cl) == WS_MAX_AUTH ) {
		// Authentication method not supported, reverting to digest
		wsman_transport_set_auth_method(cl, "digest");
	}
}

void OpenWsmanClient::SetUserName(const char *user_name)
{
	if (user_name) {
		wsman_transport_set_userName(cl, (char*)user_name);
	}
}
void OpenWsmanClient::SetPassword(const char *password)
{
       if (password) {
	   	wsman_transport_set_password(cl, (char*)password);
       }
}
void OpenWsmanClient::SetEncoding(const char *encoding)
{
	if(encoding) {
		wsmc_set_encoding(cl,(char *)encoding);
	}
}
void OpenWsmanClient::SetNamespace(const char *ns)
{
	if(ns) {
		wsmc_set_namespace(cl, (char *)ns);
	}
}

void OpenWsmanClient::SetProxy(const char *proxy, const char *proxy_username, const char *proxy_password)
{
        if (proxy) {
                wsman_transport_set_proxy(cl, (char*)proxy);
        }

        if (proxy_username) {
                wsman_transport_set_proxy_username(cl, (char*)proxy_username);
        }

        if (proxy_password) {
                wsman_transport_set_proxy_password(cl, (char*)proxy_password);
        }
}

#ifdef _WIN32
void OpenWsmanClient::SetClientCert(const char *oid, const char *cert, const bool local)
{
	if (cert) {
		wsman_transport_set_cainfo(cl, (char*)cert);
	}

	if (oid) {
		wsman_transport_set_caoid(cl, (char*)oid);
	}

	wsman_transport_set_calocal(cl, local);
}


#else
// Set server certificate params
// params: cainfo - string naming a file holding one or more certificates to verify the peer with.
//         capath - string naming a dierctory holding multiple CA certificates to verify the peer with.
// Give null arguments if you want curl to search for certificates inthe default path
// 
void OpenWsmanClient::SetServerCert(const char *cainfo, const char *capath)
{
	// This means curl verifies the server certificate
	wsman_transport_set_verify_peer(cl, 1);

	// This means the certificate must indicate that the server is the server to which you meant to connect.
	wsman_transport_set_verify_host(cl, 2);

	if (cainfo) {
		wsman_transport_set_cainfo(cl, (char*)cainfo);
	}
	if (capath) {
		wsman_transport_set_capath(cl, (char*)capath);
	}

}

// Set client certificates params
// params: cert - file name of your certificate.
//         key  - file name of your private key.
void OpenWsmanClient::SetClientCert(const char *cert, const char *key)
{
	if (cert) { 
		wsman_transport_set_cert(cl, (char*)cert);
	}
	if (key) {
		wsman_transport_set_key(cl, (char*)key);
	}
}
#endif
