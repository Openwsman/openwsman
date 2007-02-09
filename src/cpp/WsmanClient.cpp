//----------------------------------------------------------------------------
//
//  Copyright (C) Intel Corporation, 2003 - 2006.
//
//  File:       WsmanClient.h 
//
//  Contents:   A wrapper class for openweman
//
//----------------------------------------------------------------------------

#include "WsmanClient.h"


extern "C" {
#include "wsman-api.h"

}

#include "wsman-client-transport.h"

#define WSMAN_ENCODING		"UTF-8"

using namespace WsmanClientNamespace;

static string	XmlDocToString			(WsXmlDocH		doc);
static void	SetDefaultOptions		(actionOptions *options);
static bool	CheckClientResponseCode	(WsManClient* cl, long &returnCode, int &lastErr, string &error);
static string GetSubString(const string& xml, const string& start, const string& end);


static string GetSubString(const string& xml, const string& start, const string& end)
{
	size_t begin = xml.find(start);
	if(begin < 0) {
		return "";
	}
	begin += start.length();
	size_t finish = xml.find(end, begin);
	if(finish < 0) {
		return "";
	}
	return xml.substr(begin, finish - begin);	
}

// Construct from params.
WsmanClient::WsmanClient(const char *endpoint,
		const	char	*cert,
		const   char *oid,
		const	char *auth_method)
{	
	cl = wsman_create_client_from_uri(endpoint);

	wsman_transport_set_auth_method ((char *)auth_method);
	if (ws_client_transport_get_auth_value() == 0 ) {
		// Authentication method not supported, reverting to digest
		wsman_transport_set_auth_method("digest");
	}

	if (cert) {
		wsman_transport_set_cafile((char*)cert);
	}

	// TO-DO: support tls and kerberos.
}

// Initialize the transport layer, this static method should be called only once.
void WsmanClient::Init()
{
	wsman_client_transport_init((void*)NULL);
}

// Destructor.
WsmanClient::~WsmanClient() 
{
	wsman_release_client(cl);
}

/// <summary>
/// Creates a new instance of a resource and returns the URI of the new object.
/// </summary>
/// <param name="resourceUri">string, The identifier of the resource to be created</param>
/// <param name="content">Xml srting representaion of the objects settings</param>
/// <returns>string - xml represantation EndpointReference of the created object</returns>
string WsmanClient::Create(const string &resourceUri, const string &data)
{
	actionOptions options;
	SetDefaultOptions(&options);

	WsXmlDocH createResponse = ws_transfer_create_fromtext(cl, 
			resourceUri.c_str(),
			options,
			data.c_str(), data.length(), WSMAN_ENCODING);

	destroy_action_options(&options);
	string error;
	long code;
	int lastErr;
	if (! CheckClientResponseCode(cl, code, lastErr, error)) {
		ws_xml_destroy_doc(createResponse);
		if(lastErr) {
			throw(TransportException(lastErr, error));
		} else {
			throw HttpException(code, error);
		}
	}
	else if (!createResponse) {
		ws_xml_destroy_doc(createResponse);
		// throw exception("WsmanClient: unknown error has occured");
	} else if (wsman_client_check_for_fault(createResponse)) {
		WsmanException e(code, error);
		GetWsmanFault(XmlDocToString(createResponse), e);
		ws_xml_destroy_doc(createResponse);
		throw e;
	}

	string xml = ExtractPayload(XmlDocToString(createResponse));
	ws_xml_destroy_doc(createResponse);
	return xml; 
}

/// <summary>
/// Delete the resource specified in the resource URI 
/// (Only if one instance of the object exists)
/// </summary>
/// <param name="resourceUri">string, The identifier of the resource to be deleted</param>
void WsmanClient::Delete(const string &resourceUri, NameValuePairs &s)
{
	actionOptions options;
	SetDefaultOptions(&options);

	// Add selectors.
	for (PairsIterator p = s.begin(); p != s.end(); ++p) {
		if(p->second != "")
			wsman_client_add_selector(&options, (char *)p->first.c_str(), (char *)p->second.c_str());
	}

	WsXmlDocH deleteResponse = ws_transfer_delete(	cl, 
			(char *)resourceUri.c_str(),
			options);
	destroy_action_options(&options);
	string error;
	long code;
	int lastErr;
	if (! CheckClientResponseCode(cl, code, lastErr, error)) {
		ws_xml_destroy_doc(deleteResponse);
		if(lastErr) {
			throw TransportException(lastErr, error);
		} else {
			throw HttpException(code, error);
		}
	} else if (!deleteResponse) {
		ws_xml_destroy_doc(deleteResponse);
		// throw exception("WsmanClient: unknown error has occured");
	} else if (wsman_client_check_for_fault(deleteResponse)) {
		WsmanException e(code, error);
		GetWsmanFault(XmlDocToString(deleteResponse), e);
		ws_xml_destroy_doc(deleteResponse);
		throw e;
	}
	ws_xml_destroy_doc(deleteResponse);
}

/// <summary>
/// Enumerate resource. 
/// </summary>
/// <param name="resourceUri">string, The identifier of the resource to be retrived</param>
/// <returns>ArrayList of strings containing xml response</returns>
void WsmanClient::Enumerate(const string &resourceUri, vector<string> &enumRes)
{
	actionOptions options;
	SetDefaultOptions(&options);
	WsXmlDocH doc;
	char *enumContext;
	WsXmlDocH enum_response = wsenum_enumerate(cl, (char *)resourceUri.c_str(),  options);

	string error;
	long code;
	int lastErr;
	if (! CheckClientResponseCode(cl, code, lastErr, error)) {
		ws_xml_destroy_doc(enum_response);
		destroy_action_options(&options);
		if(lastErr) {
			throw(TransportException(lastErr, error));
		} else {
			throw HttpException(code, error);
		}
	} else if (!enum_response) {
		destroy_action_options(&options);
		ws_xml_destroy_doc(enum_response);
		// throw exception("WsmanClient: unknown error has occured");
	} else if (wsman_client_check_for_fault(enum_response)) {
		WsmanException e(code, error);
		GetWsmanFault(XmlDocToString(enum_response), e);
		ws_xml_destroy_doc(enum_response);
		destroy_action_options(&options);
		if(e.GetFaultSubcode().compare("EndpointUnavailable") != 0) {
			throw e;
		} else {
			return;
		}
	}
	enumContext = wsenum_get_enum_context(enum_response);
	ws_xml_destroy_doc(enum_response);

	while (enumContext != NULL && enumContext[0] != 0 ) {
		doc = wsenum_pull(cl, resourceUri.c_str(), options, enumContext);
		// Check for success (500,400 are OK??)
		if (! CheckClientResponseCode(cl, code, lastErr, error)) {
			ws_xml_destroy_doc(doc);
			destroy_action_options(&options);
			if(lastErr) {
				throw(TransportException(lastErr, error));
			} else {
				throw HttpException(code, error);
			}
		}
		else if (!doc) {
			ws_xml_destroy_doc(doc);
			destroy_action_options(&options);
			// throw exception("WsmanClient: unknown error has occured");
		} else if (wsman_client_check_for_fault(doc)) {
			WsmanException e(code, error);
			GetWsmanFault(XmlDocToString(doc), e);
			destroy_action_options(&options);
			ws_xml_destroy_doc(doc);
			throw e;			
		}
		enumRes.push_back(ExtractItems((XmlDocToString(doc))));
		enumContext = wsenum_get_enum_context(doc);    
	}
	destroy_action_options(&options);
}

/// <summary>
/// Retrieves the resource specified by 'resource' and returns an XML representation 
/// of the current instance of the resource. 
/// </summary>
/// <param name="resourceUri">string, The identifier of the resource to be retrieved</param>
/// <param name="s">Selectors, selectors to identify the requested object</param>
/// <returns>WsXmlDocH, an XML representation of the instance. </returns>
string WsmanClient::Get(const string &resourceUri, NameValuePairs *s)
{
	actionOptions options;
	SetDefaultOptions(&options);
	WsXmlDocH doc;
	// Add selectors.
	if (s) {
		for (PairsIterator p = s->begin(); p != s->end(); ++p) {
			if(p->second != "")	wsman_client_add_selector(&options, (char *)p->first.c_str(), (char *)p->second.c_str());
		}
	}
	doc = ws_transfer_get(cl, (char *)resourceUri.c_str(), options);

	destroy_action_options(&options);
	string error;
	long code;
	int lastErr;
	if (! CheckClientResponseCode(cl, code, lastErr, error)) {
		ws_xml_destroy_doc(doc);
		if (lastErr) {
			throw(TransportException(lastErr, error));
		} else {
			throw HttpException(code, error);
		}
	} else if (!doc) {
		ws_xml_destroy_doc(doc);
		//throw exception("WsmanClient: unknown error has occured");
	} else if (wsman_client_check_for_fault(doc)) {
		WsmanException e(code, error);
		GetWsmanFault(XmlDocToString(doc), e);
		ws_xml_destroy_doc(doc);
		throw e;
	}
	string xml = ExtractPayload(XmlDocToString(doc));
	ws_xml_destroy_doc(doc);
	return xml;
}

/// <summary>
/// Update a resource.
/// </summary>
/// <param name="resourceUri">string, The identifier of the resource to update</param>
/// <param name="content">string, xml string holding the updated resource content</param>
/// <param name="s">Selectors, selectors to identify the requested object</param>
/// <returns>The URI of the updated resource.</returns>
//???? why is string returned
string WsmanClient::Put(const string &resourceUri, const string &content, NameValuePairs &s)
{
	actionOptions options;
	SetDefaultOptions(&options);
	WsXmlDocH doc;

	// Add selectors.
	for (PairsIterator p = s.begin(); p != s.end(); ++p) {
		if(p->second != "")
			wsman_client_add_selector(&options, (char *)p->first.c_str(), (char *)p->second.c_str());
	}

	doc = ws_transfer_put_fromtext(cl, resourceUri.c_str(), options, content.c_str(), content.length(), WSMAN_ENCODING);

	destroy_action_options(&options);
	string error;
	long code;
	int lastErr;
	if (! CheckClientResponseCode(cl, code, lastErr, error)) {
		ws_xml_destroy_doc(doc);
		if(lastErr) {
			throw(TransportException(lastErr, error));
		} else {
			throw HttpException(code, error);
		}
	}
	else if (!doc) {
		ws_xml_destroy_doc(doc);
		// throw exception("WsmanClient: unknown error has occured");
	} else if (wsman_client_check_for_fault(doc)) {
		WsmanException e(code, error);
		GetWsmanFault(XmlDocToString(doc), e);
		ws_xml_destroy_doc(doc);
		throw e;
	}
	string xml = ExtractPayload(XmlDocToString(doc));
	ws_xml_destroy_doc(doc);
	return xml;
}

/// <summary>
/// Invokes a method and returns the results of the method call.
/// </summary>
/// <param name="resourceUri">string, The identifier of the resource</param>
/// <param name="methodName">string, The method name to be invoked</param>
/// <param name="inputParams">string, The XML representation of the input for the method.</param>
/// <returns>The XML representation of the method output.</returns>
string WsmanClient::Invoke(const string &resourceUri, const string &methodName, const string &content, NameValuePairs &s)
{
	actionOptions options;
	SetDefaultOptions(&options);
	WsXmlDocH doc;
	string error;

	// Add selectors.
	for (PairsIterator p = s.begin(); p != s.end(); ++p) {
		if(p->second != "")	wsman_client_add_selector(&options, (char *)p->first.c_str(), (char *)p->second.c_str());
	}

	doc = wsman_invoke_fromtext(cl, resourceUri.c_str(), options,
			(char *)methodName.c_str(), content.c_str(),
			content.length(), WSMAN_ENCODING);

	destroy_action_options(&options);
	long code;
	int lastErr;
	if (! CheckClientResponseCode(cl, code, lastErr, error)) {
		ws_xml_destroy_doc(doc);
		if(lastErr) {
			throw(TransportException(lastErr, error));
		} else {
			throw HttpException(code, error);
		}
	} else if (!doc) {
		ws_xml_destroy_doc(doc);
		//throw exception("WsmanClient: unknown error has occured");
	} else if (wsman_client_check_for_fault(doc)) {
		WsmanException e(code, error);
		GetWsmanFault(XmlDocToString(doc), e);
		ws_xml_destroy_doc(doc);
		throw e;
	}
	string xml = ExtractPayload(XmlDocToString(doc));
	ws_xml_destroy_doc(doc);
	return xml;
}


string WsmanClient::ExtractPayload(string xml)
{

	WsXmlDocH  doc = wsman_client_read_memory(cl, (char *)xml.c_str(),
			xml.length(), NULL, 0);
	WsXmlNodeH bodyNode = ws_xml_get_soap_body(doc);	  
	WsXmlNodeH payloadNode = ws_xml_get_child(bodyNode, 0, NULL, NULL);
	char *buf = wsman_client_node_to_buf( payloadNode);
	string payload = string(buf);
	u_free(buf);
	return payload;
}

string WsmanClient::ExtractItems(string xml)
{

	WsXmlDocH  doc = wsman_client_read_memory(cl, (char *)xml.c_str(),
			xml.length(), NULL, 0);
	WsXmlNodeH bodyNode = ws_xml_get_soap_body(doc);
	WsXmlNodeH pullResponse = ws_xml_get_child(bodyNode, 0, XML_NS_ENUMERATION, WSENUM_PULL_RESP);
	WsXmlNodeH itemsNode = ws_xml_get_child(pullResponse, 0, XML_NS_ENUMERATION, WSENUM_ITEMS);
	char *buf = wsman_client_node_to_buf( ws_xml_get_child(itemsNode, 0 , NULL, NULL ));
	string payload = string(buf);
	u_free(buf);
	return payload;
}



void WsmanClient::GetWsmanFault(string xml, WsmanException &e)
{
	WsManFault *fault = wsman_client_fault_new();
	WsXmlDocH  doc = wsman_client_read_memory(cl, (char *)xml.c_str(),
			xml.length(), NULL, 0);

	wsman_client_get_fault_data(doc, fault);

	string subcode_s = string(fault->subcode);	
	string code_s = string(fault->code);
	string reason_s = string(fault->reason);
	string detail_s = string(fault->fault_detail);

	e.SetWsmanFaultFields(code_s, subcode_s, reason_s, detail_s);
}



/// <summary>
/// Convert XML doc to string
/// </summary>
/// <param name="doc">XML doc to convert</param>
string XmlDocToString(WsXmlDocH doc) {
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

/// <summary>
/// Set default request options.
/// </summary>
void SetDefaultOptions(actionOptions *options)
{
	initialize_action_options(options);
	//options->max_envelope_size = OPEN_WSMAN_MAX_ENVELOPE_SIZE;
}

/// <summary>
/// Check for errors.
/// </summary>
bool CheckClientResponseCode(WsManClient* cl, long &responseCode, int &lastError, string &error)
{
	responseCode = wsman_client_get_response_code(cl);
	lastError = wsman_client_get_last_error(cl);
	if (lastError) {
		error = "Failed to establish a connection with service\n";
#ifdef _WIN32
		error.append("Last Windows ");
#endif 
		error.append("Error Code = ");
		char buffer[65];
		snprintf(buffer,sizeof(buffer),"%d",lastError);
		error.append(buffer);
		error.append("\n");		
		return false;
	}
	if (responseCode == 200) {
		error = "WsmanClient recieved: 200 HTTP OK\n"; 
		return true;
	}
	if (responseCode == 400) {
		error = "WsmanClient recieved: 400 Bad Request\n" \
			 "The request could not be understood by the server due to malformed syntax.\n";
		return true;
	} else if (responseCode == 500) {
		error = "WsmanClient recieved: 500 Internal Server Error\n" \
			 "The server encountered an unexpected condition which prevented it from fulfilling the request.\n"; 
		return true;
	}
	error = "Failed to establish a connection with service\nHttp Error = ";
	char buffer[65];
	snprintf(buffer,sizeof(buffer),"%d",responseCode);
	error.append(buffer);
	error.append("\n");	
	return false;		
}
