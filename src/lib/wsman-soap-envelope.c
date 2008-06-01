/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Anas Nashif
 * @author Liang Hou
 */

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-client-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-faults.h"
#include "wsman-soap-envelope.h"
#include "wsman-epr.h"


/**
 * Change Endpoint Reference from request to response format
 * @param dstHeader Destination header
 * @param epr The Endpoint Reference
 */
static void
wsman_epr_from_request_to_response(WsXmlNodeH dstHeader, WsXmlNodeH epr)
{
	int i;
	WsXmlNodeH child;
	WsXmlNodeH node = !epr ? NULL : ws_xml_get_child(epr, 0,
							 XML_NS_ADDRESSING,
							 WSA_ADDRESS);
	ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_TO,
			 !node ? WSA_TO_ANONYMOUS :
			 ws_xml_get_node_text(node));

	if (!epr)
		goto cleanup;

	if ((node = ws_xml_get_child(epr, 0, XML_NS_ADDRESSING,
			      WSA_REFERENCE_PROPERTIES))) {
		for (i = 0; (child = ws_xml_get_child(node, i, NULL, NULL)) != NULL; i++) {
			ws_xml_duplicate_tree(dstHeader, child);
		}
	}
	if ((node = ws_xml_get_child(epr, 0, XML_NS_ADDRESSING,
			      WSA_REFERENCE_PARAMETERS))) {
		for (i = 0; (child = ws_xml_get_child(node, i, NULL, NULL)) != NULL; i++) {
			ws_xml_duplicate_tree(dstHeader, child);
		}
	}

cleanup:
	return;
}

/**
 * Create a response SOAP envelope
 * @param rqstDoc The XML document of the request
 * @param action the Response action
 * @return Response envelope
 */
WsXmlDocH
wsman_create_response_envelope(WsXmlDocH rqstDoc, const char *action)
{

	WsXmlDocH doc = ws_xml_create_envelope();
	WsXmlNodeH dstHeader, srcHeader, srcNode;
	if (wsman_is_identify_request(rqstDoc))
		return doc;
	if (!doc)
		return NULL;

	dstHeader = ws_xml_get_soap_header(doc);
	srcHeader = ws_xml_get_soap_header(rqstDoc);

	srcNode = ws_xml_get_child(srcHeader, 0, XML_NS_ADDRESSING,
			     WSA_REPLY_TO);
	wsman_epr_from_request_to_response(dstHeader, srcNode);

	if (action != NULL) {
		ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_ACTION,
				 action);
	} else {
		if ((srcNode = ws_xml_get_child(srcHeader, 0, XML_NS_ADDRESSING,
				      WSA_ACTION)) != NULL) {
			if ((action = ws_xml_get_node_text(srcNode)) != NULL) {
				size_t len = strlen(action) + sizeof(WSFW_RESPONSE_STR) + 2;
				char *tmp = (char *) u_malloc(sizeof(char) * len);
				if (tmp && action) {
					sprintf(tmp, "%s%s", action, WSFW_RESPONSE_STR);
					ws_xml_add_child(dstHeader, XML_NS_ADDRESSING,
							 WSA_ACTION, tmp);
					u_free(tmp);
				}
			}
		}
	}

	if ((srcNode = ws_xml_get_child(srcHeader, 0, XML_NS_ADDRESSING,
					WSA_MESSAGE_ID)) != NULL) {
		ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_RELATES_TO,
				 ws_xml_get_node_text(srcNode));
	}
	return doc;
}

/**
 * Check Identify Request
 * @param buf Message buffer
 * @return 1 if true, 0 if not
 */
int wsman_check_identify(WsmanMessage * msg)
{
	int ret = 0;
	WsXmlDocH doc = ws_xml_read_memory( u_buf_ptr(msg->request),
					   u_buf_len(msg->request), msg->charset,  0);

	if (doc == NULL) {
		return 0;
	}
	if (wsman_is_identify_request(doc)) {
		ret = 1;
	}
	ws_xml_destroy_doc(doc);
	return ret;
}

/**
 * Buid Inbound Envelope
 * @param buf Message buffer
 * @return XML document with Envelope
 */
WsXmlDocH wsman_build_inbound_envelope(WsmanMessage * msg)
{
	WsXmlDocH doc = ws_xml_read_memory( u_buf_ptr(msg->request),
					   u_buf_len(msg->request), msg->charset,  0);

	if (doc == NULL) {
		wsman_set_fault(msg, WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0, NULL);
		return NULL;
	}
	if (wsman_is_identify_request(doc)) {
		wsman_set_message_flags(msg, FLAG_IDENTIFY_REQUEST);
	}
	wsman_is_valid_envelope(msg, doc);
	return doc;
}

/**
 * Get SOAP header value
 * @param fw SOAP Framework handle
 * @param doc XML document
 * @param nsUri Namespace URI
 * @param name Header element name
 * @return Header value
 */
char *wsman_get_soap_header_value( WsXmlDocH doc, const char *nsUri, const char *name)
{
	char *retVal = NULL;
	WsXmlNodeH node = wsman_get_soap_header_element( doc, nsUri, name);

	if (node != NULL)
		retVal = u_str_clone(ws_xml_get_node_text(node));

	return retVal;
}


/**
 * Get SOAP Header
 * @param doc XML document
 * @param nsUri Namespace URI
 * @param name Header element name
 * @return XML node
 */
WsXmlNodeH
wsman_get_soap_header_element(WsXmlDocH doc, const char *nsUri, const char *name)
{
	WsXmlNodeH node = ws_xml_get_soap_header(doc);
	if (node && name) {
		node = ws_xml_find_in_tree(node, nsUri, name, 1);
	}
	return node;
}


/**
 * Build SOAP Fault
 * @param soapNsUri SOAP Namespace URI
 * @param faultNsUri Fault Namespace URI
 * @param code Fault code
 * @param subCode Fault Subcode
 * @param reason Fault Reson
 * @param detail Fault Details
 * @return Fault XML document
 */
WsXmlDocH
wsman_build_soap_fault(const char *soapNsUri, const char *faultNsUri,
		       const char *code, const char *subCode, const char *reason,
		       const char *detail)
{
	WsXmlDocH doc;

	if (faultNsUri == NULL)
		faultNsUri = soapNsUri;

	if ((doc = ws_xml_create_doc( soapNsUri, SOAP_ENVELOPE)) != NULL) {
		WsXmlNodeH node, root, fault, body;
		root = ws_xml_get_doc_root(doc);
		body = ws_xml_add_child(root, soapNsUri, SOAP_BODY, NULL);

		ws_xml_define_ns(root, soapNsUri, NULL, 0);
		ws_xml_define_ns(root, XML_NS_ADDRESSING, NULL, 0);
		ws_xml_define_ns(root, XML_NS_XML_NAMESPACES, NULL, 0);
		if (strcmp(soapNsUri, faultNsUri) != 0)
			ws_xml_define_ns(root, faultNsUri, NULL, 0);
		if (body
		    && (fault = ws_xml_add_child(body, soapNsUri, SOAP_FAULT,
					 NULL))) {
			if (code != NULL
			    && (node = ws_xml_add_child(fault, soapNsUri,
						 SOAP_CODE,
						 NULL)) != NULL) {
				ws_xml_add_qname_child(node, soapNsUri,
						       SOAP_VALUE,
						       soapNsUri, code);

				if (subCode != NULL
				    &&
				    (node = ws_xml_add_child(node, soapNsUri,
						      SOAP_SUBCODE,
						      NULL)) != NULL) {
					ws_xml_add_qname_child(node,
							       soapNsUri,
							       SOAP_VALUE,
							       faultNsUri,
							       subCode);
				}
			}
			if (reason && (node = ws_xml_add_child(fault, soapNsUri,
						 SOAP_REASON, NULL))) {
				node = ws_xml_add_child(node, soapNsUri,
						     SOAP_TEXT, reason);
				ws_xml_add_node_attr(node,
						     XML_NS_XML_NAMESPACES,
						     SOAP_LANG, "en");
			}
			if (detail) {
				ws_xml_add_child(fault, soapNsUri, SOAP_DETAIL, detail);
			}
		}
	}
	return doc;
}




/**
 * Check if Envelope is valid
 * @param  msg Message data
 * @param doc XML document
 * @return 1 if envelope is valid, 0 if not
 */
int wsman_is_valid_envelope(WsmanMessage * msg, WsXmlDocH doc)
{
	int retval = 1;
	char *soapNsUri;
	WsXmlNodeH header;
	WsXmlNodeH root = ws_xml_get_doc_root(doc);

	if (strcmp(SOAP_ENVELOPE, ws_xml_get_node_local_name(root)) != 0) {
		wsman_set_fault(msg,
				WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0,
				"No Envelope");
		retval = 0;
		debug("no envelope");
		goto cleanup;
	}
	soapNsUri = ws_xml_get_node_name_ns(root);
	if (strcmp(soapNsUri, XML_NS_SOAP_1_2) != 0) {
		wsman_set_fault(msg, SOAP_FAULT_VERSION_MISMATCH, 0, NULL);
		retval = 0;
		debug("version mismatch");
		goto cleanup;
	}
	if (ws_xml_get_soap_body(doc) == NULL) {
		wsman_set_fault(msg,
				WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0,
				"No Body");
		retval = 0;
		debug("no body");
		goto cleanup;
	}
	header = ws_xml_get_soap_header(doc);
	if (!header) {
		wsman_set_fault(msg,
				WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0,
				"No Header");
		retval = 0;
		debug("no header");
		goto cleanup;
	} else {
		if (!wsman_is_identify_request(doc) && !wsman_is_event_related_request(doc)) {
			WsXmlNodeH resource_uri =
			    ws_xml_get_child(header, 0,
					     XML_NS_WS_MAN,
					     WSM_RESOURCE_URI);
			WsXmlNodeH action = ws_xml_get_child(header, 0,
							     XML_NS_ADDRESSING,
							     WSA_ACTION);
			WsXmlNodeH reply = ws_xml_get_child(header, 0,
							    XML_NS_ADDRESSING,
							    WSA_REPLY_TO);
			WsXmlNodeH to = ws_xml_get_child(header, 0,
							 XML_NS_ADDRESSING,
							 WSA_TO);
			if (!resource_uri) {
				wsman_set_fault(msg,
						WSA_DESTINATION_UNREACHABLE,
						WSMAN_DETAIL_INVALID_RESOURCEURI,
						NULL);
				retval = 0;
				debug("no wsman:ResourceURI");
				goto cleanup;
			}
			if (!action) {
				wsman_set_fault(msg,
						WSA_ACTION_NOT_SUPPORTED,
						0, NULL);
				retval = 0;
				debug("no wsa:Action");
				goto cleanup;
			}
			if (!reply) {
				wsman_set_fault(msg,
						WSA_MESSAGE_INFORMATION_HEADER_REQUIRED,
						0, NULL);
				retval = 0;
				debug("no wsa:ReplyTo");
				goto cleanup;
			}
			if (!to) {
				wsman_set_fault(msg,
						WSA_DESTINATION_UNREACHABLE,
						0, NULL);
				retval = 0;
				debug("no wsa:To");
				goto cleanup;
			}
		}
	}
      cleanup:
	return retval;
}


/**
 * Check if Envelope is valid
 * @param  msg Message data
 * @param doc XML document
 * @return 1 if envelope is valid, 0 if not
 */
int wsman_is_valid_xml_envelope(WsXmlDocH doc)
{
	int retval = 1;
	char *soapNsUri;
	WsXmlNodeH root = ws_xml_get_doc_root(doc);

	if (strcmp(SOAP_ENVELOPE, ws_xml_get_node_local_name(root)) != 0) {
		retval = 0;
		goto cleanup;
	}
	soapNsUri = ws_xml_get_node_name_ns(root);
	if (strcmp(soapNsUri, XML_NS_SOAP_1_2) != 0) {
		retval = 0;
		goto cleanup;
	}
	if (ws_xml_get_soap_body(doc) == NULL) {
		retval = 0;
		goto cleanup;
	}
      cleanup:
	return retval;
}


/**
 * Create a Fault
 * @param rqstDoc Request document (Envelope)
 * @param code Fault code
 * @param subCodeNs Namespace of sub code
 * @param subCode Sub code
 * @param lang Language for Reason section
 * @param reason Fault Reason
 * @param addDetailProc Callback for details
 * @param addDetailProcData Pointer to callback data
 * @return XML document of the fault
 */
WsXmlDocH
wsman_create_fault_envelope(WsXmlDocH rqstDoc,
			    const char *code,
			    const char *subCodeNs,
			    const char *subCode,
			    const char *fault_action,
			    const char *lang, const char *reason, const char *faultDetail)
{
	WsXmlDocH doc = NULL;
	WsXmlNodeH header, body, fault, codeNode, node;

	char uuidBuf[50];
	char *soapNs;

	if (rqstDoc) {
		doc = wsman_create_response_envelope(rqstDoc, fault_action);
	} else {
		/* FIXME */
		doc = ws_xml_create_envelope();
	}

	if (doc == NULL) {
		return NULL;
	}
	header = ws_xml_get_soap_header(doc);
	body = ws_xml_get_soap_body(doc);
	soapNs = ws_xml_get_node_name_ns(body);
	fault = ws_xml_add_child(body, soapNs, SOAP_FAULT, NULL);
	codeNode = ws_xml_add_child(fault, soapNs, SOAP_CODE, NULL);
	node = ws_xml_add_child(codeNode, soapNs, SOAP_VALUE, NULL);

	ws_xml_set_node_qname_val(node, soapNs, code);

	if (subCode && subCode[0] != 0 ) {
		node =
		    ws_xml_add_child(codeNode, soapNs, SOAP_SUBCODE, NULL);
		node = ws_xml_add_child(node, soapNs, SOAP_VALUE, NULL);
		if (subCodeNs)
			ws_xml_set_node_qname_val(node, subCodeNs,
						  subCode);
		else
			ws_xml_set_node_text(node, subCode);
	}
	if (reason) {
		node = ws_xml_add_child(fault, soapNs, SOAP_REASON, NULL);
		node = ws_xml_add_child(node, soapNs, SOAP_TEXT, NULL);
		ws_xml_set_node_text(node, reason);
		ws_xml_set_node_lang(node, !lang ? "en" : lang);
	}
	if (faultDetail) {
		WsXmlNodeH d =
		    ws_xml_add_child(fault, soapNs, SOAP_DETAIL, NULL);
		node =
		    ws_xml_add_child_format(d, XML_NS_WS_MAN,
					    SOAP_FAULT_DETAIL, "%s/%s",
					    XML_NS_WSMAN_FAULT_DETAIL,
					    faultDetail);
	}
	generate_uuid(uuidBuf, sizeof(uuidBuf), 0);
	ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_MESSAGE_ID,
			 uuidBuf);

	return doc;
}

static int interpretxpath(char **xpath)
{
	return 1;
}

int wsman_parse_enum_request(WsContextH cntx,
		WsEnumerateInfo * enumInfo)
{
	filter_t *filter = NULL;
	WsXmlNodeH node;
	WsXmlDocH doc = cntx->indoc;
	if (!doc)
		return 0;

	node = ws_xml_get_soap_body(doc);
	if (node && (node = ws_xml_get_child(node, 0,
					XML_NS_ENUMERATION,
					WSENUM_ENUMERATE))) {

		WsXmlNodeH opt = ws_xml_get_child(node, 0, XML_NS_WS_MAN,
				WSM_ENUM_MODE);
		/* Enumeration mode */
		if (opt) {
			char *text = ws_xml_get_node_text(opt);
			if (text != NULL) {
				if (strcmp(text, WSM_ENUM_EPR) == 0)
					enumInfo->flags |= WSMAN_ENUMINFO_EPR;
				else if (strcmp(text, WSM_ENUM_OBJ_AND_EPR) == 0)
					enumInfo->flags |= WSMAN_ENUMINFO_OBJEPR;
			}
		}

		/* Polymorphism */
		opt = ws_xml_get_child(node, 0, XML_NS_CIM_BINDING,
				WSMB_POLYMORPHISM_MODE);
		if (opt) {
			char *mode = ws_xml_get_node_text(opt);
			if (strcmp(mode, WSMB_EXCLUDE_SUBCLASS_PROP) == 0) {
				enumInfo->flags |= WSMAN_ENUMINFO_POLY_EXCLUDE;
			} else if (strcmp(mode, WSMB_INCLUDE_SUBCLASS_PROP) == 0) {
				enumInfo->flags |= WSMAN_ENUMINFO_POLY_INCLUDE;
			} else if (strcmp(mode, WSMB_NONE) == 0) {
				enumInfo->flags |= WSMAN_ENUMINFO_POLY_NONE;
			}
		} else {
			enumInfo->flags |= WSMAN_ENUMINFO_POLY_INCLUDE;
		}

		/* Enum Optimization */
		opt = ws_xml_get_child(node, 0, XML_NS_WS_MAN,
				WSM_OPTIMIZE_ENUM);
		if (opt) {
			WsXmlNodeH max = ws_xml_get_child(node, 0,
					XML_NS_WS_MAN,
					WSM_MAX_ELEMENTS);
			enumInfo->flags |= WSMAN_ENUMINFO_OPT;
			if (max) {
				char *text = ws_xml_get_node_text(max);
				if (text != NULL) {
					enumInfo->maxItems = atoi(text);
				}
			} else {
				enumInfo->maxItems = 1;
			}
		}

		/* Filter */
		filter = filter_deserialize(node);
		enumInfo->filter = filter;
		if(filter) {
			if(strcmp(filter->dialect, WSM_ASSOCIATION_FILTER_DIALECT) == 0) {
				if(filter->assocType == 0)
					enumInfo->flags |= WSMAN_ENUMINFO_ASSOC;
				else
					enumInfo->flags |= WSMAN_ENUMINFO_REF;
			}
			else if(strcmp(filter->dialect, WSM_CQL_FILTER_DIALECT) ==0)
				enumInfo->flags |= WSMAN_ENUMINFO_CQL;
			else if(strcmp(filter->dialect, WSM_WQL_FILTER_DIALECT) == 0)
				enumInfo->flags |= WSMAN_ENUMINFO_WQL;
			else if(strcmp(filter->dialect, WSM_SELECTOR_FILTER_DIALECT) == 0)
				enumInfo->flags |= WSMAN_ENUMINFO_SELECTOR;
			else {
				if(interpretxpath(&filter->query))
					enumInfo->flags |= WSMAN_ENUMINFO_WQL;
				else
					return 0;

			}
		}
	}

	return 1;
}

static int is_existing_filter_epr(WsXmlNodeH node, filter_t **f)
{
	char *uri;
	WsXmlNodeH xmlnode = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_RESOURCE_URI);
	if(xmlnode == NULL)
		return -1;
	uri = ws_xml_get_node_text(xmlnode);
	if(strcmp(uri, CIM_ALL_AVAILABLE_CLASSES) == 0)
		return -1;
	xmlnode = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);
	if(xmlnode == NULL)
		return -1;
	*f = u_zalloc(sizeof(filter_t));
	return 0;
}

int wsman_parse_credentials(WsXmlDocH doc, WsSubscribeInfo * subsInfo,
		WsmanFaultCodeType *faultcode,
		WsmanFaultDetailType *detailcode)
{
	int i = 0;
	WsXmlNodeH tnode = NULL, snode = NULL, node = NULL, temp = NULL;
	char *value = NULL;
	snode = ws_xml_get_soap_header(doc);
	snode = ws_xml_get_child(snode, 0, XML_NS_TRUST, WST_ISSUEDTOKENS);
	if(snode == NULL) return 0;
	tnode = ws_xml_get_child(snode, i, XML_NS_TRUST, WST_REQUESTSECURITYTOKENRESPONSE);
	while(tnode)
	{
		i++;
		node = ws_xml_get_child(tnode, 0, XML_NS_POLICY, WSP_APPLIESTO);
		if(node) {
			node = ws_xml_get_child(node, 0, XML_NS_ADDRESSING, WSA_EPR);
			if(node) {
				node = ws_xml_get_child(node, 0, XML_NS_ADDRESSING, WSA_ADDRESS);
				if(node)
					if(strcmp(ws_xml_get_node_text(node), subsInfo->epr_notifyto)) {
						*faultcode = WSMAN_INVALID_PARAMETER;
						*detailcode = WSMAN_DETAIL_INVALID_ADDRESS;
						return -1;
					}
			}
		}
		node = ws_xml_get_child(tnode, 0, XML_NS_TRUST, WST_TOKENTYPE);
		value = ws_xml_get_node_text(node);
		if(strcmp(value, WST_USERNAMETOKEN) == 0) {
			node = ws_xml_get_child(tnode, 0, XML_NS_TRUST, WST_REQUESTEDSECURITYTOKEN);
			if(node) {
				node = ws_xml_get_child(node, 0, XML_NS_SE, WSSE_USERNAMETOKEN);
				if(node) {
					temp = ws_xml_get_child(node, 0, XML_NS_SE, WSSE_USERNAME);
					if(temp)
						subsInfo->username = u_strdup(ws_xml_get_node_text(temp));
					temp = ws_xml_get_child(node, 0, XML_NS_SE, WSSE_PASSWORD);
					if(temp)
						subsInfo->password = u_strdup(ws_xml_get_node_text(temp));
				}
			}
			debug("subsInfo->username = %s, subsInfo->password = %s", subsInfo->username, \
				subsInfo->password);
		}
		else if(strcmp(value, WST_CERTIFICATETHUMBPRINT) == 0) {
			node = ws_xml_get_child(tnode, 0, XML_NS_TRUST, WST_REQUESTEDSECURITYTOKEN);
			if(node) {
				node = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_CERTIFICATETHUMBPRINT);
				if(node)
					subsInfo->certificate_thumbprint = u_strdup(ws_xml_get_node_text(node));
			}
		}
		else {
			*faultcode = WSMAN_INVALID_OPTIONS;
			*detailcode = WST_DETAIL_UNSUPPORTED_TOKENTYPE;
			return -1;
		}
		tnode = ws_xml_get_child(snode, i, XML_NS_TRUST, WST_REQUESTSECURITYTOKENRESPONSE);
	}
	return 0;
}

int
wsman_parse_event_request(WsXmlDocH doc, WsSubscribeInfo * subsInfo,
		WsmanFaultCodeType *faultcode,
		WsmanFaultDetailType *detailcode)
{
	WsXmlNodeH node;
	filter_t *f = NULL;
	if (!doc)
		return 0;

	node = ws_xml_get_soap_body(doc);
	if (node && (node = ws_xml_get_child(node, 0,
					XML_NS_EVENTING,
					WSEVENT_SUBSCRIBE))) {
		f = filter_deserialize(node);
		subsInfo->filter = f;
		if(f) {
			if(strcmp(f->dialect,WSM_CQL_FILTER_DIALECT) == 0)
				subsInfo->flags |= WSMAN_SUBSCRIPTION_CQL;
			else if(strcmp(f->dialect, WSM_WQL_FILTER_DIALECT) == 0)
				subsInfo->flags |= WSMAN_SUBSCRIPTION_WQL;
			else {
				*faultcode = WSE_FILTERING_NOT_SUPPORTED;
					return -1;
			}
		} else {
			if(is_existing_filter_epr(ws_xml_get_soap_header(doc), &f)) {
				*faultcode = WSE_FILTERING_NOT_SUPPORTED;
				return -1;
			} else {
				subsInfo->flags |= WSMAN_SUBSCRIPTION_SELECTORSET;
			}
		}
	}

	return 0;
}

char *
wsman_get_option_set(WsContextH cntx, WsXmlDocH doc,
		const char *op)
{
	char *optval = NULL;
	int index = 0;
	WsXmlNodeH node, option;
	if (doc == NULL) {
		doc = cntx->indoc;
		if (!doc)
			return NULL;
	}

	node = ws_xml_get_soap_header(doc);
	if (node && (node = ws_xml_get_child(node, 0,
					XML_NS_WS_MAN, WSM_OPTION_SET))) {
		while ((option = ws_xml_get_child(node, index++, XML_NS_WS_MAN,
						WSM_OPTION))) {
			char *attrVal = ws_xml_find_attr_value(option, NULL,
					WSM_NAME);
			if (attrVal && strcmp(attrVal, op ) == 0 ) {
				optval = ws_xml_get_node_text(option);
				if (optval[0] == 0)
					optval = u_strdup_printf("true");
				debug("Option: %s=%s", attrVal, optval);
				break;
			}
		}
	}
	return optval;
}



int wsman_get_max_elements(WsContextH cntx, WsXmlDocH doc)
{
	int max_elements = 1;
	if (doc == NULL)
		doc = cntx->indoc;

	if (doc) {
		WsXmlNodeH node = ws_xml_get_soap_body(doc);

		if (node && (node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION,
					 WSENUM_PULL))) {
			node = ws_xml_get_child(node, 0, XML_NS_WS_MAN,
					     WSENUM_MAX_ELEMENTS);
			if (node) {
				char *text = ws_xml_get_node_text(node);
				if (text != NULL)
					max_elements = atoi(text);
			}
		}
	} else {
		return 0;
	}
	return max_elements;

}

unsigned long wsman_get_max_envelope_size(WsContextH cntx, WsXmlDocH doc)
{
	unsigned long size = 0;
	WsXmlNodeH header, maxsize;
	char *mu = NULL;
	if (doc == NULL)
		doc = cntx->indoc;
	header = ws_xml_get_soap_header(doc);
	maxsize = ws_xml_get_child(header, 0, XML_NS_WS_MAN,
			     WSM_MAX_ENVELOPE_SIZE);
	mu = ws_xml_find_attr_value(maxsize, XML_NS_SOAP_1_2,
				    SOAP_MUST_UNDERSTAND);
	if (mu != NULL && strcmp(mu, "true") == 0) {
		size = ws_deserialize_uint32(NULL, header,
					     0, XML_NS_WS_MAN,
					     WSM_MAX_ENVELOPE_SIZE);
	}
	return size;
}

char * wsman_get_fragment_string(WsContextH cntx, WsXmlDocH doc)
{
	WsXmlNodeH header, n;
	char *mu = NULL;
	if(doc == NULL)
		doc = cntx->indoc;
	header = ws_xml_get_soap_header(doc);
	n = ws_xml_get_child(header, 0, XML_NS_WS_MAN,
			     WSM_FRAGMENT_TRANSFER);
	if (n != NULL) {
		mu = ws_xml_find_attr_value(n, XML_NS_SOAP_1_2,
					    SOAP_MUST_UNDERSTAND);
		if (mu != NULL && strcmp(mu, "true") == 0) {
			return ws_xml_get_node_text(n);
		}
	}
	return NULL;
}


void wsman_get_fragment_type(char *fragstr, int *fragment_flag, char **element,
	int *index)
{
	char *p, *p1, *p2, *dupstr;
	*fragment_flag = 0;
	*element = NULL;
	*index = 0;
	if(fragstr == NULL) return;
	dupstr = u_strdup(fragstr);
	p = strstr(dupstr, "/text()");
	if(p) {
		*p = '\0';
		*fragment_flag = 2;
		*element = u_strdup(dupstr);
	}
	else {
		if((p1 = strstr(dupstr, "[")) && (p2 = strstr(dupstr, "]"))) {
			*element = u_strndup(dupstr, p1 - dupstr);
			*p2 = '\0';
			*index = atoi(p1+1);
			*fragment_flag = 3;
		}
		else {
			*element = u_strdup(dupstr);
			*fragment_flag = 1;
		}
	}
	u_free(dupstr);
}


char *wsman_get_method_name(WsContextH cntx)
{
	char *m, *method = NULL;
	char *tmp = NULL;
	m = wsman_get_action(cntx, NULL);
	if (m != NULL && m[0] != 0) {
		tmp = strrchr(m, '/');
		if(tmp)
			method = u_strdup(tmp + 1);
		debug("method or action: %s", method);
	}
	return method;
}



char *wsman_get_class_name(WsContextH cntx)
{
	char *className = NULL;
	char *resource_uri = wsman_get_resource_uri(cntx, NULL);
	char *tmp = NULL;
	if(resource_uri) {
		tmp = strrchr(resource_uri, '/');
		if(tmp)
			className = u_strdup(tmp + 1);
	}
	return className;
}




char *
wsman_get_resource_uri(WsContextH cntx, WsXmlDocH doc)
{
	char *val = NULL;
	WsXmlNodeH header, node;

	if (doc == NULL) {
		doc = cntx->indoc;
		if (!doc)
			return NULL;
	}

	header = ws_xml_get_soap_header(doc);
	node = ws_xml_get_child(header, 0, XML_NS_WS_MAN,
				WSM_RESOURCE_URI);
	val = (!node) ? NULL : ws_xml_get_node_text(node);
	return val;
}


hash_t *
wsman_get_method_args(WsContextH cntx, const char *resource_uri)
{
	char *input = NULL;
	hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);
	WsXmlDocH doc = cntx->indoc;
	WsXmlNodeH in_node, epr;
	WsXmlNodeH body = ws_xml_get_soap_body(doc);
	char *mn = NULL;
	selector_entry *sentry;
	debug("wsman_get_method_args");

	if (!doc) {
		error("xml document is null");
		goto cleanup;
	}

	mn = wsman_get_method_name(cntx);
	input = u_strdup_printf("%s_INPUT", mn);
	in_node = ws_xml_get_child(body, 0, resource_uri, input);
	if (!in_node) {
		char *xsd = u_strdup_printf("%s.xsd", resource_uri);
		in_node = ws_xml_get_child(body, 0, xsd, input);
		u_free(xsd);
	}
	if (in_node) {
		WsXmlNodeH arg;
		int index = 0;
		while ((arg = ws_xml_get_child(in_node, index++, NULL, NULL))) {
			char *key = ws_xml_get_node_local_name(arg);

			sentry = u_malloc(sizeof(*sentry));
			epr = ws_xml_get_child(arg, 0, XML_NS_ADDRESSING,
					WSA_REFERENCE_PARAMETERS);
			if (epr) {
				sentry->type = 1;
				sentry->entry.eprp = epr_deserialize(arg, NULL, NULL, 0);
				if (!hash_alloc_insert(h, key, sentry)) {
					error("hash_alloc_insert failed");
				}
			} else {
				debug("text: %s", key);
				sentry->type = 0;
				sentry->entry.text = ws_xml_get_node_text(arg);
				if (!hash_alloc_insert(h, key, sentry)) {
					error("hash_alloc_insert failed");
				}
			}
		}
	}
	u_free(mn);
	u_free(input);


	if (!hash_isempty(h))
		return h;
cleanup:
	hash_destroy(h);
	return NULL;
}



hash_t *
wsman_get_selectors_from_epr(WsContextH cntx, WsXmlNodeH epr_node)
{
	WsXmlNodeH selector, node, epr;
	selector_entry *sentry;
	int index = 0;
	hash_t *h = hash_create2(HASHCOUNT_T_MAX, 0, 0);

	node = ws_xml_get_child(epr_node, 0, XML_NS_WS_MAN,
			WSM_SELECTOR_SET);
	if (!node) {
		debug("no SelectorSet defined");
		hash_destroy(h);
		return NULL;
	}
	while ((selector =
		ws_xml_get_child(node, index++, XML_NS_WS_MAN,  WSM_SELECTOR))) {
		char *attrVal =
		    ws_xml_find_attr_value(selector, XML_NS_WS_MAN,
					   WSM_NAME);
		if (attrVal == NULL)
			attrVal = ws_xml_find_attr_value(selector, NULL,
						   WSM_NAME);

		if (attrVal && !hash_lookup(h, attrVal)) {
			sentry = u_malloc(sizeof(*sentry));
			epr = ws_xml_get_child(selector, 0, XML_NS_ADDRESSING,
					WSA_EPR);
			if (epr) {
				debug("epr: %s", attrVal);
				sentry->type = 1;
				sentry->entry.eprp = epr_deserialize(selector, XML_NS_ADDRESSING,
					WSA_EPR, 1);
				if (!hash_alloc_insert(h, attrVal, sentry)) {
					error("hash_alloc_insert failed");
				}
			} else {
				debug("text: %s", attrVal);
				sentry->type = 0;
				sentry->entry.text = ws_xml_get_node_text(selector);
				if (!hash_alloc_insert(h, attrVal,
						sentry)) {
					error("hash_alloc_insert failed");
				}
			}
		}
	}

	if (!hash_isempty(h))
		return h;

	hash_destroy(h);
	return NULL;
}

hash_t *
wsman_get_selector_list(WsContextH cntx, WsXmlDocH doc)
{
	WsXmlNodeH header;
	hash_t *h = NULL;

	if (doc == NULL) {
		doc = cntx->indoc;
		if (!doc)
			return NULL;
	}
	header = ws_xml_get_soap_header(doc);
	if (header) {
		h = wsman_get_selectors_from_epr(cntx, header);
	}
	return h;
}

hash_t *
wsman_get_selector_list_from_filter(WsContextH cntx, WsXmlDocH doc)
{
	WsXmlNodeH body;
	WsXmlNodeH node, assInst, object;

	if (doc == NULL) {
		doc = cntx->indoc;
		if (!doc)
			return NULL;
	}

	body = ws_xml_get_soap_body(doc);
	node = ws_xml_get_child(body, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE);
	if(!node) {
		debug("no SelectorSet defined. Missing Enumerate");
		return NULL;
	}
	node = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSENUM_FILTER);
	if(!node) {
		debug("no SelectorSet defined. Missing Filter");
		return NULL;
	}

	assInst = ws_xml_get_child(node, 0, XML_NS_CIM_BINDING, WSMB_ASSOCIATION_INSTANCES);
	if(!assInst) {
		assInst = ws_xml_get_child(node, 0, XML_NS_CIM_BINDING, WSMB_ASSOCIATED_INSTANCES);
		if(!assInst) {
			debug("no SelectorSet defined. Missing AssociationInstances / AssociatedInstances");
			return NULL;
		}
	}

	object = ws_xml_get_child(assInst, 0, XML_NS_CIM_BINDING, WSMB_OBJECT);
	if(!node) {
		debug("no SelectorSet defined. Missing Object");
		return NULL;
	}

	node = ws_xml_get_child(object, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);
	if(!node) {
		debug("no SelectorSet defined. Missing ReferenceParameters");
		return NULL;
	}

	return  wsman_get_selectors_from_epr(cntx, node);
}


char *
wsman_get_selector(WsContextH cntx,
			 WsXmlDocH doc, const char *name, int index)
{
	char *val = NULL;
	if (doc == NULL)
		doc = cntx->indoc;
	if (doc) {
		WsXmlNodeH header = ws_xml_get_soap_header(doc);
		WsXmlNodeH node = ws_xml_get_child(header, index, XML_NS_WS_MAN,
				     WSM_SELECTOR_SET);

		if (node) {
			WsXmlNodeH selector;
			int index = 0;

			while ((selector = ws_xml_get_child(node, index++,
						 XML_NS_WS_MAN, WSM_SELECTOR))) {
				char *attrVal = ws_xml_find_attr_value(selector,
							   XML_NS_WS_MAN,
							   WSM_NAME);
				if (attrVal == NULL)
					attrVal = ws_xml_find_attr_value(selector, NULL, WSM_NAME);

				if (attrVal && !strcmp(attrVal, name)) {
					val = ws_xml_get_node_text(selector);
					break;
				}
			}
		}
	}
	debug("Selector value for %s: %s", name, val);
	return val;
}

char *wsman_get_action(WsContextH cntx, WsXmlDocH doc)
{
	char *val = NULL;
	if (doc == NULL) {
		doc = cntx->indoc;
	}
	if (doc) {
		WsXmlNodeH header = ws_xml_get_soap_header(doc);
		WsXmlNodeH node = ws_xml_get_child(header, 0, XML_NS_ADDRESSING,
				     WSA_ACTION);
		val = (!node) ? NULL : ws_xml_get_node_text(node);
	}
	return val;
}




void wsman_add_selector(WsXmlNodeH baseNode, const char *name, const char *val)
{
	WsXmlNodeH selector = NULL;
	WsXmlDocH epr = NULL;
	WsXmlNodeH set = ws_xml_get_child(baseNode, 0, XML_NS_WS_MAN,
			WSM_SELECTOR_SET);

	if (val && strstr(val, WSA_EPR)) {
		epr = ws_xml_read_memory(val, strlen(val), NULL, 0);
	}

	if (set || (set = ws_xml_add_child(baseNode,
				XML_NS_WS_MAN, WSM_SELECTOR_SET, NULL))) {
		if (epr) {
			if ((selector = ws_xml_add_child(set, XML_NS_WS_MAN, WSM_SELECTOR,
							NULL))) {
				ws_xml_duplicate_tree(selector, ws_xml_get_doc_root(epr));
				ws_xml_add_node_attr(selector, NULL, WSM_NAME,
						name);
			}
		} else {
			if ((selector = ws_xml_add_child(set, XML_NS_WS_MAN, WSM_SELECTOR,
							val))) {
				ws_xml_add_node_attr(selector, NULL, WSM_NAME,
						name);
			}
		}
	}
	return;
}



void
wsman_set_estimated_total(WsXmlDocH in_doc,
			  WsXmlDocH out_doc, WsEnumerateInfo * enumInfo)
{
	WsXmlNodeH header = ws_xml_get_soap_header(in_doc);
	if (ws_xml_get_child(header, 0,
			     XML_NS_WS_MAN, WSM_REQUEST_TOTAL) != NULL) {
		if (out_doc) {
			WsXmlNodeH response_header =
			    ws_xml_get_soap_header(out_doc);
			if (enumInfo->totalItems >= 0)
				ws_xml_add_child_format(response_header,
							XML_NS_WS_MAN,
							WSM_TOTAL_ESTIMATE,
							"%d",
							enumInfo->
							totalItems);
		}
	}
	return;
}



void wsman_add_namespace_as_selector(WsXmlDocH doc, const char *_namespace)
{
	WsXmlNodeH header = ws_xml_get_soap_header(doc);
	wsman_add_selector(header, CIM_NAMESPACE_SELECTOR, _namespace);

	return;
}


void wsman_add_fragement_for_header(WsXmlDocH indoc, WsXmlDocH outdoc)
{
	WsXmlNodeH inheader, outheader;
	WsXmlNodeH fragmentnode;
	inheader = ws_xml_get_soap_header(indoc);
	fragmentnode = ws_xml_get_child(inheader, 0, XML_NS_WS_MAN, WSM_FRAGMENT_TRANSFER);
	if(fragmentnode == NULL)
		return;
	outheader = ws_xml_get_soap_header(outdoc);
	ws_xml_duplicate_tree(outheader, fragmentnode);
}


int wsman_is_identify_request(WsXmlDocH doc)
{
	WsXmlNodeH node = ws_xml_get_soap_body(doc);
	node = ws_xml_get_child(node, 0, XML_NS_WSMAN_ID, WSMID_IDENTIFY);
	if (node)
		return 1;
	else
		return 0;
}

int wsman_is_event_related_request(WsXmlDocH doc)
{
	WsXmlNodeH node = ws_xml_get_soap_header(doc);
	char *action = NULL;
	node = ws_xml_get_child(node, 0, XML_NS_ADDRESSING, WSA_ACTION);
	action = ws_xml_get_node_text(node);
	if (!action)
		return 0;

	if(strcmp(action, EVT_ACTION_UNSUBSCRIBE) ==0 || strcmp(action, EVT_ACTION_RENEW) ==0
		|| strcmp(action, EVT_ACTION_PULL) == 0)
		return 1;
	else
		return 0;
}

int time_expired(unsigned long lt)
{
	struct timeval tv;
	if(lt == 0) return 0; // 0 means it never expires
	gettimeofday(&tv, NULL);
	if(!(tv.tv_sec< lt))
		return 1;
	else
		return 0;
}

void
wsman_set_expiretime(WsXmlNodeH  node,
                    unsigned long * expire,
                    WsmanFaultCodeType *fault_code)
{
	struct timeval  tv;
	time_t timeout;
	char *text;
	XML_DATETIME tmx;
	gettimeofday(&tv, NULL);
	text = ws_xml_get_node_text(node);
	*fault_code = WSMAN_DETAIL_OK;
	if (text == NULL) {
		*fault_code = WSEN_INVALID_EXPIRATION_TIME;
		return;
	}
	debug("wsen:Expires = %s", text);
	if (text[0] == 'P') {
		//  xml duration
		if (ws_deserialize_duration(text, &timeout)) {
			*fault_code = WSEN_INVALID_EXPIRATION_TIME;
			goto DONE;
		}
		*expire = tv.tv_sec + timeout;
		goto DONE;
	}

	// timeout is XML datetime type
	if (ws_deserialize_datetime(text, &tmx)) {
		*fault_code = WSEN_UNSUPPORTED_EXPIRATION_TYPE;
		goto DONE;
	}
	timeout = mktime(&(tmx.tm)) + 60*tmx.tz_min;
	*expire = timeout;
DONE:
	return;
}

WsXmlDocH
wsman_create_doc(const char *rootname)
{
	return ws_xml_create_doc(NULL, (char *)rootname);
}

void
wsman_destroy_doc(WsXmlDocH doc)
{
	ws_xml_destroy_doc(doc);
}

