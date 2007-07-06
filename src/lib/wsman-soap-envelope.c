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
 * @author Sumeet Kukreja, Dell Inc.
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

	if ((node =
	     ws_xml_get_child(epr, 0, XML_NS_ADDRESSING,
			      WSA_REFERENCE_PROPERTIES))) {
		for (i = 0;
		     (child =
		      ws_xml_get_child(node, i, NULL, NULL)) != NULL;
		     i++) {
			ws_xml_duplicate_tree(dstHeader, child);
		}
	}
	if ((node =
	     ws_xml_get_child(epr, 0, XML_NS_ADDRESSING,
			      WSA_REFERENCE_PARAMETERS))) {
		for (i = 0;
		     (child =
		      ws_xml_get_child(node, i, NULL, NULL)) != NULL;
		     i++) {
			ws_xml_duplicate_tree(dstHeader, child);
		}
	}
      cleanup:
	return;
}

/**
 * Create a response SOAP envelope
 * @param cntx Context
 * @param rqstDoc The XML document of the request
 * @param action the Response action
 * @return Response envelope
 */
WsXmlDocH
wsman_create_response_envelope(WsContextH cntx,
			       WsXmlDocH rqstDoc, const char *action)
{
	SoapH soap = cntx->soap;
	char *soapNs =
	    ws_xml_get_node_name_ns(ws_xml_get_doc_root(rqstDoc));
	WsXmlDocH doc = ws_xml_create_envelope(soap, soapNs);
	WsXmlNodeH dstHeader, srcHeader, srcNode;
	if (wsman_is_identify_request(rqstDoc))
		return doc;
	if (!doc)
		return NULL;

	dstHeader = ws_xml_get_soap_header(doc);
	srcHeader = ws_xml_get_soap_header(rqstDoc);

	srcNode =
	    ws_xml_get_child(srcHeader, 0, XML_NS_ADDRESSING,
			     WSA_REPLY_TO);
	wsman_epr_from_request_to_response(dstHeader, srcNode);

	if (action != NULL) {
		ws_xml_add_child(dstHeader, XML_NS_ADDRESSING, WSA_ACTION,
				 action);
	} else {
		if ((srcNode =
		     ws_xml_get_child(srcHeader, 0, XML_NS_ADDRESSING,
				      WSA_ACTION)) != NULL) {
			if ((action =
			     ws_xml_get_node_text(srcNode)) != NULL) {
				size_t len =
				    strlen(action) +
				    sizeof(WSFW_RESPONSE_STR) + 2;
				char *tmp =
				    (char *) u_malloc(sizeof(char) * len);
				if (tmp) {
					sprintf(tmp, "%s%s", action,
						WSFW_RESPONSE_STR);
					ws_xml_add_child(dstHeader,
							 XML_NS_ADDRESSING,
							 WSA_ACTION, tmp);
					u_free(tmp);
				}
			}
		}
	}

	if ((srcNode = ws_xml_get_child(srcHeader, 0,
					XML_NS_ADDRESSING,
					WSA_MESSAGE_ID)) != NULL) {
		ws_xml_add_child(dstHeader, XML_NS_ADDRESSING,
				 WSA_RELATES_TO,
				 ws_xml_get_node_text(srcNode));
	}
	return doc;
}


/**
 * Buid Inbound Envelope
 * @param  fw SOAP Framework handle
 * @param buf Message buffer
 * @return XML document with Envelope
 */
WsXmlDocH wsman_build_inbound_envelope(SoapH soap, WsmanMessage * msg)
{
	WsXmlDocH doc = ws_xml_read_memory(soap, u_buf_ptr(msg->request),
					   u_buf_len(msg->request), NULL,
					   0);

	if (doc == NULL) {
		wsman_set_fault(msg,
				WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0,
				NULL);
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
char *wsman_get_soap_header_value(SoapH soap,
				  WsXmlDocH doc, const char *nsUri, const char *name)
{
	char *retVal = NULL;
	WsXmlNodeH node =
	    wsman_get_soap_header_element(soap, doc, nsUri, name);

	if (node != NULL)
		retVal = u_str_clone(ws_xml_get_node_text(node));

	return retVal;
}


/**
 * Get SOAP Header
 * @param fw SOAP Framework handle
 * @param doc XML document
 * @param nsUri Namespace URI
 * @param name Header element name
 * @return XML node
 */
WsXmlNodeH
wsman_get_soap_header_element(SoapH soap,
			      WsXmlDocH doc, const char *nsUri, const char *name)
{
	WsXmlNodeH node = ws_xml_get_soap_header(doc);
	if (node && name) {
		node = ws_xml_find_in_tree(node, nsUri, name, 1);
	}
	return node;
}


/**
 * Build SOAP Fault
 * @param  fw SOAP Framework handle
 * @param soapNsUri SOAP Namespace URI
 * @param faultNsUri Fault Namespace URI
 * @param code Fault code
 * @param subCode Fault Subcode
 * @param reason Fault Reson
 * @param detail Fault Details
 * @return Fault XML document
 */
WsXmlDocH
wsman_build_soap_fault(SoapH soap, const char *soapNsUri, const char *faultNsUri,
		       const char *code, const char *subCode, const char *reason,
		       const char *detail)
{
	WsXmlDocH doc;

	if (faultNsUri == NULL)
		faultNsUri = soapNsUri;

	if ((doc =
	     ws_xml_create_doc(soap, soapNsUri, SOAP_ENVELOPE)) != NULL) {
		WsXmlNodeH node;
		WsXmlNodeH fault;
		WsXmlNodeH root = ws_xml_get_doc_root(doc);
		//WsXmlNodeH header = WsXmlAddChild(root, soapNsUri, SOAP_HEADER, NULL);
		WsXmlNodeH body =
		    ws_xml_add_child(root, soapNsUri, SOAP_BODY, NULL);

		ws_xml_define_ns(root, soapNsUri, NULL, 0);
		ws_xml_define_ns(root, XML_NS_ADDRESSING, NULL, 0);
		ws_xml_define_ns(root, XML_NS_XML_NAMESPACES, NULL, 0);
		if (strcmp(soapNsUri, faultNsUri) != 0)
			ws_xml_define_ns(root, faultNsUri, NULL, 0);
		if (body
		    && (fault =
			ws_xml_add_child(body, soapNsUri, SOAP_FAULT,
					 NULL))) {
			if (code != NULL
			    && (node =
				ws_xml_add_child(fault, soapNsUri,
						 SOAP_CODE,
						 NULL)) != NULL) {
				ws_xml_add_qname_child(node, soapNsUri,
						       SOAP_VALUE,
						       soapNsUri, code);

				if (subCode != NULL
				    &&
				    (node =
				     ws_xml_add_child(node, soapNsUri,
						      SOAP_SUBCODE,
						      NULL)) != NULL) {
					ws_xml_add_qname_child(node,
							       soapNsUri,
							       SOAP_VALUE,
							       faultNsUri,
							       subCode);
				}
			}
			if (reason
			    && (node =
				ws_xml_add_child(fault, soapNsUri,
						 SOAP_REASON, NULL))) {
				node =
				    ws_xml_add_child(node, soapNsUri,
						     SOAP_TEXT, reason);
				ws_xml_add_node_attr(node,
						     XML_NS_XML_NAMESPACES,
						     SOAP_LANG, "en");
			}
			if (detail)
				ws_xml_add_child(fault, soapNsUri,
						 SOAP_DETAIL, detail);
		}
	}
	return doc;
}




/**
 * Buid SOAP Version Mismtach Fault
 * @param  fw SOAP Framework handle
 * @todo Send fault back
 */
#if 0
void wsman_build_soap_version_fault(SoapH soap)
{
	WsXmlDocH fault =
	    wsman_build_soap_fault(soap, NULL, XML_NS_SOAP_1_2,
				   "VersionMismatch", NULL,
				   "Version Mismatch", NULL);
	if (fault != NULL) {
		WsXmlNodeH upgrade;
		WsXmlNodeH h = ws_xml_get_soap_header(fault);

		ws_xml_define_ns(ws_xml_get_doc_root(fault),
				 XML_NS_SOAP_1_1, NULL, 0);

		upgrade =
		    ws_xml_add_child(h, XML_NS_SOAP_1_2, SOAP_UPGRADE,
				     NULL);
		if (upgrade) {
			WsXmlNodeH node;

			if ((node = ws_xml_add_child(upgrade,
						     XML_NS_SOAP_1_2,
						     SOAP_SUPPORTED_ENVELOPE,
						     NULL))) {
				ws_xml_add_qname_attr(node, NULL, "qname",
						      XML_NS_SOAP_1_2,
						      SOAP_ENVELOPE);
			}
			node = ws_xml_add_child(upgrade, XML_NS_SOAP_1_2,
						SOAP_SUPPORTED_ENVELOPE,
						NULL);
			if (node) {
				ws_xml_add_qname_attr(node, NULL, "qname",
						      XML_NS_SOAP_1_1,
						      SOAP_ENVELOPE);
			}
		}
		// FIXME:       Send fault
		ws_xml_destroy_doc(fault);
	}
}

#endif


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
		if (!wsman_is_identify_request(doc)) {
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
 * @param cntx WS Context
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
wsman_create_fault_envelope(WsContextH cntx,
			    WsXmlDocH rqstDoc,
			    const char *code,
			    const char *subCodeNs,
			    const char *subCode,
			    const char *fault_action,
			    const char *lang, const char *reason, const char *faultDetail)
{
	WsXmlDocH doc = NULL;
	WsXmlNodeH header;
	WsXmlNodeH body;
	WsXmlNodeH fault;
	WsXmlNodeH codeNode;
	WsXmlNodeH node;
	char uuidBuf[50];
	char *soapNs;
	if (rqstDoc) {
		doc =
		    wsman_create_response_envelope(cntx, rqstDoc,
						   fault_action);
	} else {
		SoapH soap = cntx->soap;
		doc = ws_xml_create_envelope(soap, NULL);
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

static void wsman_parse_xpath_filter( WsContextH cntx,
		WsXmlNodeH filter, 
		WsEnumerateInfo * enumInfo)
{
	filter_t *f = (filter_t *)u_zalloc(sizeof(filter_t));
	f->xpath = u_strdup(ws_xml_get_node_text(filter));
	debug("Xpath filter: %s", f->xpath );
	enumInfo->filter = f;
}



static void wsman_parse_assoc_filter( WsContextH cntx,
			WsXmlNodeH filter, 
			WsEnumerateInfo * enumInfo)
{
	WsXmlNodeH node;
	WsXmlNodeH fnode;
	epr_t *epr = NULL;
	filter_t *f = NULL;
	if ((node = ws_xml_get_child(filter, 0, XML_NS_CIM_BINDING,
				WSMB_ASSOCIATION_INSTANCES) ) != NULL) 	
		enumInfo->flags |= WSMAN_ENUMINFO_REF;
	else if ( (node = ws_xml_get_child(filter, 0, XML_NS_CIM_BINDING,
				WSMB_ASSOCIATED_INSTANCES)) != NULL) 	
		enumInfo->flags |= WSMAN_ENUMINFO_ASSOC;

	if (node == NULL) {
		return;
	}

	epr = wsman_get_epr(cntx, node, WSMB_OBJECT, XML_NS_CIM_BINDING );
	f = (filter_t *)u_zalloc(sizeof(filter_t));
	f->epr = epr;
	fnode = ws_xml_get_child(node, 0, XML_NS_CIM_BINDING, WSMB_RESULT_CLASS_NAME);
	if (fnode) 
		f->resultClass = u_strdup(ws_xml_get_node_text(fnode));
	else
		f->resultClass = NULL;
	fnode = ws_xml_get_child(node, 0, XML_NS_CIM_BINDING, WSMB_ROLE);
	if (fnode) 
		f->role = u_strdup(ws_xml_get_node_text(fnode));
	else
		f->role = NULL;
	fnode = ws_xml_get_child(node, 0, XML_NS_CIM_BINDING, WSMB_ASSOCIATION_CLASS_NAME);
	if (fnode)
		f->assocClass = u_strdup(ws_xml_get_node_text(fnode));
	else 
		f->assocClass = NULL;
	fnode = ws_xml_get_child(node, 0, XML_NS_CIM_BINDING, WSMB_RESULT_ROLE);
	if (fnode) 
		f->resultRole = u_strdup(ws_xml_get_node_text(fnode));
	else
		f->resultRole = NULL;
	enumInfo->filter = f;
}


int wsman_parse_enum_request(WsContextH cntx, 
		WsEnumerateInfo * enumInfo)
{
	WsXmlNodeH node;
	WsXmlDocH doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
	if (!doc) 
		return 0;

	node = ws_xml_get_soap_body(doc);
	if (node && (node = ws_xml_get_child(node, 0,
					XML_NS_ENUMERATION,
					WSENUM_ENUMERATE))) {
		WsXmlNodeH filter = ws_xml_get_child(node,
				0, XML_NS_WS_MAN, WSM_FILTER);
		WsXmlNodeH opt = ws_xml_get_child(node, 0, XML_NS_WS_MAN,
				WSM_ENUM_MODE);
		// Enumeration mode
		if (opt) {
			char *text = ws_xml_get_node_text(opt);
			if (text != NULL) {
				if (strcmp(text, WSM_ENUM_EPR) == 0)
					enumInfo->flags |= WSMAN_ENUMINFO_EPR;
				else if (strcmp(text, WSM_ENUM_OBJ_AND_EPR) == 0)
					enumInfo->flags |= WSMAN_ENUMINFO_OBJEPR;
			}
		}

		// Polymorphism
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

		// Enum Optimization
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

		// Filter
		if (filter) {
			char *attrVal = ws_xml_find_attr_value(filter, 
					NULL, WSM_DIALECT);
			if ( attrVal && strcmp(attrVal,WSM_ASSOCIATION_FILTER_DIALECT) == 0 ) {
				wsman_parse_assoc_filter(cntx, filter, enumInfo);
			}
			if (( attrVal && strcmp(attrVal,WSM_XPATH_FILTER_DIALECT) == 0 ) || !attrVal ) {
				wsman_parse_xpath_filter(cntx, filter, enumInfo);
			}
		} else {
			enumInfo->filter = NULL;
		}
	}
	return 1;
}


char* wsman_get_option_set(WsContextH cntx, WsXmlDocH doc,
		const char *op)
{
	char *optval = NULL;
	int index = 0;
	WsXmlNodeH node, option;
	if (doc == NULL) {
		doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
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
	int max_elements = 0;
	if (doc == NULL)
		doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);

	if (doc) {
		WsXmlNodeH node = ws_xml_get_soap_body(doc);

		if (node
		    && (node =
			ws_xml_get_child(node, 0, XML_NS_ENUMERATION,
					 WSENUM_PULL))) {
			node =
			    ws_xml_get_child(node, 0, XML_NS_ENUMERATION,
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


char *wsman_get_method_name(WsContextH cntx)
{
	char *m, *method = NULL;
	m = wsman_get_action(cntx, NULL);
	if (m != NULL && m[0] != 0) {
		method = u_strdup(strrchr(m, '/') + 1);
		debug("method or action: %s", method);
	}
	return method;
}



char *wsman_get_class_name(WsContextH cntx)
{
	char *resource_uri = wsman_get_resource_uri(cntx, NULL);
	char *className = u_strdup(strrchr(resource_uri, '/') + 1);
	return className;
}




char *wsman_get_resource_uri(WsContextH cntx, WsXmlDocH doc)
{
	char *val = NULL;
	WsXmlNodeH header, node;

	if (doc == NULL) {
		doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
		if (!doc)
			return NULL;
	}

	header = ws_xml_get_soap_header(doc);
	node = ws_xml_get_child(header, 0, XML_NS_WS_MAN,
				WSM_RESOURCE_URI);
	val = (!node) ? NULL : ws_xml_get_node_text(node);
	return val;
}


hash_t *wsman_get_method_args(WsContextH cntx, const char *resource_uri)
{
	char *input = NULL;
	hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);
	WsXmlDocH doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
	if (doc) {
		WsXmlNodeH in_node;
		WsXmlNodeH body = ws_xml_get_soap_body(doc);
		char *mn = wsman_get_method_name(cntx);
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
			while ((arg =
				ws_xml_get_child(in_node, index++, NULL,
						 NULL))) {
				char *key =
				    ws_xml_get_node_local_name(arg);
				if (!hash_alloc_insert
				    (h, key, ws_xml_get_node_text(arg))) {
					error("hash_alloc_insert failed");
				}
			}
		}
		u_free(mn);
		u_free(input);
	} else {
		error("xml document is null");
	}
	if (!hash_isempty(h))
		return h;

	hash_destroy(h);
	return NULL;
}



hash_t *wsman_get_selectors_from_epr(WsXmlNodeH epr_node)
{
	WsXmlNodeH selector, node;
	int index = 0;
	hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);

	node = ws_xml_get_child(epr_node, 0, XML_NS_WS_MAN, 
			WSM_SELECTOR_SET);
	if (!node) {
		debug("no SelectorSet defined");
		hash_destroy(h);
		return NULL;
	}
	while ((selector =
		ws_xml_get_child(node, index++, XML_NS_WS_MAN,
				 WSM_SELECTOR))) {
		char *attrVal =
		    ws_xml_find_attr_value(selector, XML_NS_WS_MAN,
					   WSM_NAME);
		if (attrVal == NULL)
			attrVal =
			    ws_xml_find_attr_value(selector, NULL,
						   WSM_NAME);

		debug("Selector: %s=%s", attrVal,
		      ws_xml_get_node_text(selector));
		if (attrVal) {
			if (!hash_lookup(h, attrVal)) {
				if (!hash_alloc_insert
				    (h, attrVal,
				     ws_xml_get_node_text(selector))) {
					error("hash_alloc_insert failed");
				}
			} else {
				error("duplicate selector");
			}
		}
	}

	if (!hash_isempty(h))
		return h;

	hash_destroy(h);
	return NULL;
}

hash_t *wsman_get_selector_list(WsContextH cntx, WsXmlDocH doc)
{
	WsXmlNodeH header;
	hash_t *h = NULL;

	if (doc == NULL) {
		doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
		if (!doc)
			return NULL;
	}

	header = ws_xml_get_soap_header(doc);
	if (header) {
		h = wsman_get_selectors_from_epr(header);
	} 
	return h;
}

hash_t *
wsman_get_selector_list_from_filter(WsContextH cntx, WsXmlDocH doc)
{
	WsXmlNodeH body;
	WsXmlNodeH node, assInst, object;

	if (doc == NULL) {
		doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
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

	return  wsman_get_selectors_from_epr(node);
}


char *wsman_get_selector(WsContextH cntx,
			 WsXmlDocH doc, const char *name, int index)
{
	char *val = NULL;
	if (doc == NULL)
		doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
	if (doc) {
		WsXmlNodeH header = ws_xml_get_soap_header(doc);
		WsXmlNodeH node =
		    ws_xml_get_child(header, index, XML_NS_WS_MAN,
				     WSM_SELECTOR_SET);

		if (node) {
			WsXmlNodeH selector;
			int index = 0;

			while ((selector =
				ws_xml_get_child(node, index++,
						 XML_NS_WS_MAN,
						 WSM_SELECTOR))) {
				char *attrVal =
				    ws_xml_find_attr_value(selector,
							   XML_NS_WS_MAN,
							   WSM_NAME);
				if (attrVal == NULL)
					attrVal =
					    ws_xml_find_attr_value
					    (selector, NULL, WSM_NAME);

				if (attrVal && !strcmp(attrVal, name)) {
					val =
					    ws_xml_get_node_text(selector);
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
	if (doc == NULL)
		doc = ws_get_context_xml_doc_val(cntx, WSFW_INDOC);
	if (doc) {
		WsXmlNodeH header = ws_xml_get_soap_header(doc);
		WsXmlNodeH node =
		    ws_xml_get_child(header, 0, XML_NS_ADDRESSING,
				     WSA_ACTION);
		val = (!node) ? NULL : ws_xml_get_node_text(node);
	}
	return val;
}




void wsman_add_selector(WsXmlNodeH baseNode, const char *name, const char *val)
{
	WsXmlNodeH selector = NULL;
	WsXmlDocH epr = NULL;
	WsXmlNodeH set =
	    ws_xml_get_child(baseNode, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);

	if (val && strstr(val, WSA_EPR)) {
		SoapH soap = ws_xml_get_doc_soap_handle(ws_xml_get_node_doc(baseNode));
		epr = ws_xml_read_memory(soap, val, strlen(val), NULL, 0);
	}

	if (set || (set = ws_xml_add_child(baseNode, 
				XML_NS_WS_MAN, WSM_SELECTOR_SET, NULL))) {
		if (epr) {
			if ((selector = ws_xml_add_child(set, XML_NS_WS_MAN, WSM_SELECTOR,
							NULL))) {
				ws_xml_duplicate_tree(selector,
					ws_xml_get_doc_root(epr));
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



int wsman_is_identify_request(WsXmlDocH doc)
{

	WsXmlNodeH node = ws_xml_get_soap_body(doc);
	node = ws_xml_get_child(node, 0, XML_NS_WSMAN_ID, WSMID_IDENTIFY);
	if (node)
		return 1;
	else
		return 0;
}

WsXmlDocH
wsman_create_doc(WsContextH cntx, const char *rootname)
{
	return ws_xml_create_doc(cntx->soap, NULL, (char *)rootname);
}

void
wsman_destroy_doc(WsXmlDocH doc)
{
	ws_xml_destroy_doc(doc);
}

