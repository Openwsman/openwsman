//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2007.
//            (C) Red Hat, Inc, 2015.
//
//  File:       OpenWsmanClient.cpp
//
//  Contents:   An implementation of the WsmanClient interface using openwsman
//
//----------------------------------------------------------------------------

#include "OpenWsmanClient.h"

#include <sstream>

extern "C" {
#include "u/libu.h"
#include "wsman-api.h"
}

#include "wsman-client-transport.h"

#define WSMAN_ENCODING		"UTF-8"

using namespace WsmanClientNamespace;

static bool CheckWsmanResponse(WsManClient* cl, WsXmlDocH& doc);
static bool ResourceNotFound(WsManClient* cl, WsXmlDocH& enumerationRes);
static string GetSubscribeContext(WsXmlDocH& doc);
static string ExtractPayload(WsXmlDocH& doc);
static string ExtractItems(WsXmlDocH& doc);

// Construct from params.

OpenWsmanClient::OpenWsmanClient(
	const string &host,
	const int port,
	const string &path,
	const string &scheme,
	const string &auth_method,
	const string &username,
	const string &password,
	 // proxy address include proxy port
	const string &proxy,
	//proxy user name
	const string &proxy_username,
	//proxy password
	const string &proxy_password
#ifdef _WIN32
	// determines which cert store to search
	,const bool local,
	// search for a client cert with this name
	const string &cert,
	// search for a cient cert with this oid
	const string &oid
#endif
)
{
	cl = wsmc_create(
		host.c_str(),
		port,
		path.c_str(),
		scheme.c_str(),
		username.c_str(),
		password.c_str());
	if (!cl) {
		string error;
		error.append("wsmc_create failed:");
		error.append("host: " + host);
		error.append("username: " + username);
		throw WsmanClientException(error.c_str(), WSMAN_GENERAL_ERROR);
	}
	SetAuth(auth_method);
#ifdef _WIN32
	SetClientCert(oid, cert, local);
#endif
	SetProxy(proxy.c_str(), proxy_username, proxy_password);
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
	WsmanOptions options;
	options.setNamespace(GetNamespace());

	WsXmlDocH identifyResponse = wsmc_action_identify(cl, options);
	CheckWsmanResponse(cl, identifyResponse);
	string xml = ExtractPayload(identifyResponse);
	ws_xml_destroy_doc(identifyResponse);
	return xml;
}

string OpenWsmanClient::Create(const string &resourceUri, const string &data) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());

	WsXmlDocH createResponse = wsmc_action_create_fromtext(cl,
			resourceUri.c_str(),
			options,
			data.c_str(), data.length(), WSMAN_ENCODING);
	CheckWsmanResponse(cl, createResponse);
	string xml = ExtractPayload(createResponse);
	ws_xml_destroy_doc(createResponse);
	return xml;
}

void OpenWsmanClient::Delete(const string &resourceUri, const NameValuePairs *s) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());
	options.addSelectors(s);

	WsXmlDocH deleteResponse = wsmc_action_delete(cl,
			resourceUri.c_str(),
			options);
	CheckWsmanResponse(cl, deleteResponse);
	ws_xml_destroy_doc(deleteResponse);
}

void OpenWsmanClient::Enumerate(
	const string &resourceUri,
	vector<string> &enumRes,
	const WsmanOptions &options,
	const WsmanFilter &filter) const
{
	WsXmlDocH doc;
	char *enumContext;
	WsXmlDocH enum_response = wsmc_action_enumerate(cl, (char *)resourceUri.c_str(),  options, filter);

	if(ResourceNotFound(cl, enum_response))
		throw WsmanResourceNotFound(resourceUri.c_str());

	enumContext = wsmc_get_enum_context(enum_response);
	ws_xml_destroy_doc(enum_response);

	while (enumContext != NULL && enumContext[0] != 0 ) {
		doc = wsmc_action_pull(cl, resourceUri.c_str(), options, NULL, enumContext);
		CheckWsmanResponse(cl, doc);
		string payload = ExtractItems(doc);

		if (payload.length() > 0)
			enumRes.push_back(payload);

		wsmc_free_enum_context(enumContext);
		enumContext = wsmc_get_enum_context(doc);
		ws_xml_destroy_doc(doc);
	}

	wsmc_free_enum_context(enumContext);
}

void OpenWsmanClient::Enumerate(
	const string &resourceUri,
	vector<string> &enumRes,
	const NameValuePairs *s) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());
	options.addSelectors(s);

	Enumerate(resourceUri, enumRes, options, WsmanFilter());
}

void OpenWsmanClient::Enumerate(
	const string &resourceUri,
	WsmanFilter &filter,
	vector<string> &enumRes) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());

	Enumerate(resourceUri, enumRes, options, filter);
}

string OpenWsmanClient::Get(
	const string &resourceUri,
	const WsmanOptions &options) const
{
	WsXmlDocH doc = wsmc_action_get(cl, (char *)resourceUri.c_str(), options);
	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Get(
	const string &resourceUri,
	const NameValuePairs *s) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());
	options.addSelectors(s);

	return Get(resourceUri, options);
}

string OpenWsmanClient::GetWithFlags(
	const string &resourceUri,
	const NameValuePairs *s,
	unsigned long flags) const
{
	WsmanOptions options(flags);
	options.setNamespace(GetNamespace());
	options.addSelectors(s);

	return Get(resourceUri, options);
}

string OpenWsmanClient::Put(
	const string &resourceUri,
	const string &content,
	const NameValuePairs *s) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());
	options.addSelectors(s);

	WsXmlDocH doc = wsmc_action_put_fromtext(
		cl,
		resourceUri.c_str(),
		options,
		content.c_str(),
		content.length(),
		WSMAN_ENCODING);

	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::PutWithFlags(
	const string &resourceUri,
	const string &content,
	const NameValuePairs *s,
	unsigned long flags) const
{
	WsmanOptions options(flags);
	options.setNamespace(GetNamespace());
	options.addSelectors(s);

	WsXmlDocH doc = wsmc_action_put_fromtext(
		cl,
		resourceUri.c_str(),
		options,
		content.c_str(),
		content.length(),
		WSMAN_ENCODING);

	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Invoke(
	const string &resourceUri,
	const string &methodName,
	const WsmanOptions &options) const
{
	WsXmlDocH doc = wsmc_action_invoke(
		cl,
		resourceUri.c_str(),
		options,
		methodName.c_str(),
		NULL);

	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Invoke(
	const string &resourceUri,
	const string &methodName,
	const string &content,
	const WsmanOptions &options) const
{
	WsXmlDocH doc = wsmc_action_invoke_fromtext(
		cl,
		resourceUri.c_str(),
		options,
		const_cast<char*>(methodName.c_str()),
		content.c_str(),
		content.length(),
		WSMAN_ENCODING);

	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Invoke(
	const string &resourceUri,
	const string &methodName,
	const string &content,
	const NameValuePairs *s) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());
	options.addSelectors(s);

	return Invoke(resourceUri, methodName, content, options);
}

string OpenWsmanClient::Subscribe(
	const string &resourceUri,
	const SubscribeInfo &info,
	string &subsContext) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());
	options.setDeliveryMode(static_cast<WsmanDeliveryMode>(info.delivery_mode));
	options.setDeliveryURI(info.delivery_uri);

	if (!info.refenceParam.empty())
		options.setReference(info.refenceParam);

	// Add selectors.
	options.addSelectors(info.selectorset);

	options.setExpires(info.expires);
	options.setHeartbeatInterval(info.heartbeat_interval);

	WsXmlDocH doc = wsmc_action_subscribe(cl, (char *)resourceUri.c_str(), options, NULL);
	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	subsContext = GetSubscribeContext(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

string OpenWsmanClient::Renew(
	const string &resourceUri,
	const string &subsContext,
	float expire,
	const NameValuePairs *s) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());
	options.setExpires(expire);
	options.addSelectors(s);

	WsXmlDocH doc = wsmc_action_renew(
		cl,
		resourceUri.c_str(),
		options,
		subsContext.c_str());

	CheckWsmanResponse(cl, doc);
	string xml = ExtractPayload(doc);
	ws_xml_destroy_doc(doc);
	return xml;
}

void OpenWsmanClient::Unsubscribe(
	const string &resourceUri,
	const string &subsContext,
	const NameValuePairs *s) const
{
	WsmanOptions options;
	options.setNamespace(GetNamespace());
	options.addSelectors(s);

	WsXmlDocH doc = wsmc_action_unsubscribe(
		cl,
		resourceUri.c_str(),
		options,
		subsContext.c_str());

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

bool CheckWsmanResponse(WsManClient* cl, WsXmlDocH& doc)
{
	long lastError = wsmc_get_last_error(cl);

	if(lastError) {
		std::stringstream ss1;
		ss1 << "Failed to establish a connection with the server." << std::endl << "Openwsman last error = " << lastError;
		ws_xml_destroy_doc(doc);
		throw WsmanClientException(ss1.str().c_str(), WSMAN_CONNECT_ERROR);
	}

	long responseCode = wsmc_get_response_code(cl);
	if (responseCode != 200 &&
			responseCode != 400 &&
			responseCode != 500)
	{
		std::stringstream ss2;
		ss2 << "An HTTP error occurred." << std::endl << "HTTP Error = " << responseCode;
		ws_xml_destroy_doc(doc);
		throw WsmanClientException(ss2.str().c_str(), WSMAN_HTTP_ERROR);
	}

	if(!doc)
		throw WsmanClientException("The Wsman response was NULL.");

	if (wsmc_check_for_fault(doc)) {
		WsManFault fault = {0};
		wsmc_get_fault_data(doc, &fault);
		string subcode_s = fault.subcode ? string(fault.subcode) : "";
		string code_s = fault.code ? string(fault.code) : "";
		string reason_s = fault.reason ? string(fault.reason) : "";
		string detail_s = fault.fault_detail ? string(fault.fault_detail) : "";
		ws_xml_destroy_doc(doc);

		std::stringstream ss3;
		ss3 << "A Soap Fault was received:" << std::endl;
		ss3 << "FaultCode: " << code_s << std::endl;
		ss3 << "FaultSubCode: " + subcode_s << std::endl;
		ss3 << "FaultReason: " + reason_s<< std::endl;
		ss3 << "FaultDetail: " + detail_s<< std::endl;
		ss3 << "HttpCode:  = " << responseCode;
		throw WsmanSoapFault(ss3.str().c_str(), code_s, subcode_s, reason_s, detail_s);
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
		return false;

	WsManFault fault = {0};
	bool ret = false;
	wsmc_get_fault_data(enumerationRes, &fault);
	string subcode_s = fault.subcode ? string(fault.subcode) : "";
	if(subcode_s.find("DestinationUnreachable") != string::npos)
		return true;

	if(!ret)
		CheckWsmanResponse(cl, enumerationRes);

	return ret;
}

void OpenWsmanClient::SetAuth(const string &auth_method)
{
	if (auth_method.empty())
		return;

	wsman_transport_set_auth_method(cl, auth_method.c_str());
	if (wsmc_transport_get_auth_value(cl) == WS_MAX_AUTH ) {
		// Authentication method not supported, reverting to digest
		wsman_transport_set_auth_method(cl, "digest");
	}
}
void OpenWsmanClient::SetTimeout(unsigned long mtime)
{
	wsman_transport_set_timeout(cl,mtime);
}

void OpenWsmanClient::SetUserName(const string &user_name)
{
	if (user_name.empty())
		return;

	wsman_transport_set_userName(cl, user_name.c_str());
}

void OpenWsmanClient::SetPassword(const string &password)
{
	if (password.empty())
		return;

	wsman_transport_set_password(cl, password.c_str());
}

void OpenWsmanClient::SetEncoding(const string &encoding)
{
	if (encoding.empty())
		return;

	wsmc_set_encoding(cl, encoding.c_str());
}

void OpenWsmanClient::SetNamespace(const string &ns)
{
	if (ns.empty())
		return;

	wsmc_set_namespace(cl, ns.c_str());
}

void OpenWsmanClient::SetProxy(
	const string &proxy,
	const string &proxy_username,
	const string &proxy_password)
{
        if (!proxy.empty())
                wsman_transport_set_proxy(cl, proxy.c_str());

        if (!proxy_username.empty())
                wsman_transport_set_proxy_username(cl, proxy_username.c_str());

        if (!proxy_password.empty())
                wsman_transport_set_proxy_password(cl, proxy_password.c_str());
}

#ifdef _WIN32
void OpenWsmanClient::SetClientCert(
	const string &caOid,
	const string &caName,
	const bool localCert)
{
	if (!caOid.empty())
		wsman_transport_set_caoid(cl, caOid.c_str());

	if (!caName.empty())
		wsman_transport_set_cainfo(cl, caName.c_str());

	wsman_transport_set_calocal(cl, localCert);
}


#else
// Set server certificate params
// params: cainfo - string naming a file holding one or more certificates to verify the peer with.
//         capath - string naming a dierctory holding multiple CA certificates to verify the peer with.
// Give empty strings if you want curl to search for certificates inthe default path
void OpenWsmanClient::SetServerCert(
	const string &cainfo,
	const string &capath)
{
	// This means curl verifies the server certificate
	wsman_transport_set_verify_peer(cl, 1);

	// This means the certificate must indicate that the server is the server to which you meant to connect.
	wsman_transport_set_verify_host(cl, 2);

	if (!cainfo.empty())
		wsman_transport_set_cainfo(cl, cainfo.c_str());

	if (!capath.empty())
		wsman_transport_set_capath(cl, capath.c_str());
}

// Set client certificates params
// params: cert - file name of your certificate.
//         key  - file name of your private key.
void OpenWsmanClient::SetClientCert(
	const string &cert,
	const string &key)
{
	if (!cert.empty())
		wsman_transport_set_cert(cl, cert.c_str());

	if (!key.empty())
		wsman_transport_set_key(cl, key.c_str());
}
#endif

string OpenWsmanClient::GetNamespace() const
{
	char *ns = wsmc_get_namespace(cl);
	return ns ? string(ns) : string();
}
