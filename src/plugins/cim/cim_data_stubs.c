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
 * Contributions from:
 * @author Sumeet Kukreja, Dell Inc.
 */

#ifdef HAVE_CONFIG_H
#include "wsman_config.h"
#endif

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "u/libu.h"


#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-client-api.h"
#include "wsman-xml-serializer.h"

#include "wsman-soap-envelope.h"
#include "wsman-soap-message.h"
#include "sfcc-interface.h"
#include "cim_data.h"


static CimClientInfo*
CimResource_Init(WsContextH cntx, char *username, char *password)
{
	char *_tmp = NULL;
	char *resource_uri = NULL;
	char *show_extensions;
	CimClientInfo *cimclient= (CimClientInfo *)u_zalloc(sizeof(CimClientInfo));
	WsmanStatus status;

	wsman_status_init(&status);
	resource_uri = wsman_get_resource_uri(cntx, NULL);
	debug ("username: %s, password: %s", username, (password)?"XXXXX":"Not Set" );

	debug("Connecting using sfcc %s frontend", get_cim_client_frontend());

	cimclient->cc = (void *)cim_connect_to_cimom(get_cim_host(),
			get_cim_port(), username, password , get_cim_client_frontend(), &status);

	if (!cimclient->cc) {
		return NULL;
	}

	cimclient->cntx = cntx;
	cimclient->namespaces = get_vendor_namespaces();
	cimclient->selectors = wsman_get_selector_list(cntx, NULL);

	cimclient->requested_class = wsman_get_class_name(cntx);

	cimclient->method = wsman_get_method_name(cntx);
	if(username) {
		cimclient->username = u_strdup(username);
	}
	if(password) {
		cimclient->password = u_strdup(password);
	}

	if (cimclient->selectors) {
		_tmp = cim_get_namespace_selector(cimclient->selectors);
	}
	if (_tmp) {
		cimclient->cim_namespace = _tmp;
	} else {
		cimclient->cim_namespace = get_cim_namespace();
	}
	if(resource_uri) {
		cimclient->resource_uri = u_strdup(resource_uri);
		cimclient->method_args = wsman_get_method_args(cntx, resource_uri );
	}
	show_extensions = wsman_get_option_set(cntx, NULL, WSMB_SHOW_EXTENSION );

	if (show_extensions && strcmp(show_extensions, "true") == 0) {
		cimclient->flags |= FLAG_CIM_EXTENSIONS;
	}
	if (get_omit_schema_optional() == 1) {
		cimclient->flags |= FLAG_CIM_SCHEMA_OPT;
	}

	u_free(show_extensions);
	return cimclient;
}

static void
CimResource_destroy(CimClientInfo *cimclient)
{
	if (!cimclient)
		return;
	if (cimclient->resource_uri)
		u_free(cimclient->resource_uri);
	if (cimclient->method)
		u_free(cimclient->method);
	if (cimclient->requested_class)
		u_free(cimclient->requested_class);
	if (cimclient->method_args) {
		hash_free(cimclient->method_args);
	}
	if (cimclient->selectors) {
		hash_free(cimclient->selectors);
		debug("selectors destroyed");
	}
	if (cimclient->username)
		u_free(cimclient->username);
	if (cimclient->password)
		u_free(cimclient->password);
	cim_release_client(cimclient);
	u_free(cimclient);
	debug("cimclient destroyed");
	return;
}




static int
verify_class_namespace(CimClientInfo *client)
{
	hscan_t hs;
	hnode_t *hn;
	int rv = 0;
	if ( strcmp( client->resource_uri, CIM_ALL_AVAILABLE_CLASSES ) ==0 ) {
		return 1;
	}
	debug("Requested Class: %s", client->requested_class );
	if ( client && (strstr(client->requested_class, "CIM") != NULL ) &&
			(strstr(client->resource_uri ,
				XML_NS_CIM_CLASS) != NULL ) &&
			 strcmp(client->method, TRANSFER_CREATE) != 0
			) {
		return 1;
	}

	if (client->requested_class && client->namespaces) {
		hash_scan_begin(&hs, client->namespaces);
		while ((hn = hash_scan_next(&hs))) {
			if ( ( strstr(client->requested_class,
							(char*) hnode_getkey(hn)) != NULL) &&
					(strstr(client->resource_uri ,
						(char*) hnode_get(hn) ) != NULL) ) {
				rv = 1;
				break;
			}
		}
	}
	return rv;
}




int
CimResource_Delete_EP( SoapOpH op,
		void* appData,
		void *opaqueData )
{
	WsmanStatus status;
	CimClientInfo *cimclient = NULL;
	SoapH soap = soap_get_op_soap(op);
	WsmanMessage *msg = wsman_get_msg_from_op(op);
	WsXmlDocH in_doc = NULL;
	WsXmlDocH doc = NULL;
	WsContextH cntx;
	debug( "Delete Endpoint Called");
	wsman_status_init(&status);

	in_doc = soap_get_op_doc(op, 1);
	cntx = ws_create_ep_context(soap, in_doc);

	if (msg) {
		cimclient = CimResource_Init(cntx,
				msg->auth_data.username,
				msg->auth_data.password );
		if (!cimclient) {
			status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status.fault_detail_code = 0;
			goto cleanup;
		}

	}
	if (!verify_class_namespace(cimclient) ) {
		status.fault_code = WSA_DESTINATION_UNREACHABLE;
		status.fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
	} else if ( (doc = wsman_create_response_envelope( in_doc, NULL)) ) {
		if (strstr(cimclient->resource_uri , XML_NS_CIM_CLASS ) != NULL) {
			cim_delete_instance_from_enum(cimclient, &status);
		} else {
			debug("no base class, getting instance directly with getInstance");
			cim_delete_instance(cimclient, &status);
		}
	}
	else if (status.fault_code != 0) {
		ws_xml_destroy_doc(doc);
		doc = wsman_generate_fault(in_doc, status.fault_code,
				status.fault_detail_code, NULL);
	}
debug("here");
cleanup:
	if (wsman_check_status(&status) != 0) {
		ws_xml_destroy_doc(doc);
		doc = wsman_generate_fault( soap_get_op_doc(op, 1),
				status.fault_code, status.fault_detail_code, NULL);
	}
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	} else {
		error("Invalid doc");
	}

	CimResource_destroy(cimclient);
	ws_destroy_context(cntx);
	return 0;
}

int
CimResource_Get_EP( SoapOpH op,
		void* appData,
		void *opaqueData )
{
	WsXmlDocH doc = NULL;
	WsmanStatus status;
	CimClientInfo *cimclient = NULL;
	char *fragstr = NULL;
	WsmanMessage *msg = wsman_get_msg_from_op(op);
	SoapH soap = soap_get_op_soap(op);

	WsXmlDocH in_doc = soap_get_op_doc(op, 1);
	WsContextH cntx = ws_create_ep_context(soap, in_doc);

	wsman_status_init(&status);
	if (msg) {
		cimclient = CimResource_Init(cntx,
				msg->auth_data.username,
				msg->auth_data.password );
		if (!cimclient) {
			status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status.fault_detail_code = 0;
			goto cleanup;
		}
	}
	if (!verify_class_namespace(cimclient) ) {
		status.fault_code = WSA_DESTINATION_UNREACHABLE;
		status.fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
		doc = wsman_generate_fault( in_doc, status.fault_code,
				status.fault_detail_code, NULL);
	} else {
		if ( (doc = wsman_create_response_envelope( in_doc, NULL)) ) {
			WsXmlNodeH body = ws_xml_get_soap_body(doc);
			fragstr = wsman_get_fragment_string(cntx, in_doc);
			if(fragstr)
				body = ws_xml_add_child(body, XML_NS_WS_MAN, WSM_XML_FRAGMENT,
				NULL);
			if (strstr(cimclient->resource_uri , XML_NS_CIM_CLASS ) != NULL) {
				cim_get_instance_from_enum(cimclient, cntx, body, fragstr, &status);
			} else {
				debug("no base class, getting instance directly with getInstance");
				cim_get_instance(cimclient, cntx, body, fragstr, &status);
			}
		}
	}
cleanup:
	if (wsman_check_status(&status) != 0) {
		ws_xml_destroy_doc(doc);
		doc = wsman_generate_fault( soap_get_op_doc(op, 1),
				status.fault_code, status.fault_detail_code, NULL);
	}
	if ( doc ) {
		soap_set_op_doc(op, doc, 0);
	} else {
		debug( "Invalid doc" );
	}

	CimResource_destroy(cimclient);
	ws_destroy_context(cntx);
	return 0;
}

int
CimResource_Custom_EP( SoapOpH op,
		void* appData,
		void *opaqueData )
{
	debug( "Custom Method Endpoint Called");
	WsXmlDocH doc = NULL, in_doc = NULL;
	CimClientInfo *cimclient = NULL;
	WsmanStatus status;
	WsmanMessage *msg = NULL;
	char *action;
	WsContextH cntx;

	wsman_status_init(&status);
	in_doc = soap_get_op_doc(op, 1);
	cntx = ws_create_ep_context(soap_get_op_soap(op), in_doc);

	msg = wsman_get_msg_from_op(op);
	action = wsman_get_action(cntx, in_doc );
	if (msg) {
		cimclient = CimResource_Init(cntx, msg->auth_data.username,
				msg->auth_data.password);
		if (!cimclient) {
			status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status.fault_detail_code = 0;
			goto cleanup;
		}
	}
	if (action && cimclient->resource_uri &&
			!strstr(action, cimclient->resource_uri)) {
		status.fault_code = WSA_ACTION_NOT_SUPPORTED;
		status.fault_detail_code = OWSMAN_NO_DETAILS;
		doc = wsman_generate_fault( in_doc, status.fault_code,
				status.fault_detail_code, NULL);
		debug("action not supported");
		goto cleanup;
	}
	if (!verify_class_namespace(cimclient) ) {
		status.fault_code = WSA_DESTINATION_UNREACHABLE;
		status.fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
		doc = wsman_generate_fault(in_doc, status.fault_code,
				status.fault_detail_code, NULL);
	} else {

		if ((doc = wsman_create_response_envelope( in_doc, NULL))) {
			WsXmlNodeH body = ws_xml_get_soap_body(doc);
			cim_invoke_method(cimclient, cntx, body, &status);
		}

		if (status.fault_code != 0) {
			ws_xml_destroy_doc(doc);
			doc = wsman_generate_fault( in_doc, status.fault_code,
					status.fault_detail_code, NULL);
		}
	}

cleanup:
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	} else {
		error("Invalid doc");
	}

	ws_destroy_context(cntx);
	CimResource_destroy(cimclient);
	return 0;
}




int
CimResource_Enumerate_EP( WsContextH cntx,
		WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	debug("CIM Enumeration");
	WsXmlDocH doc;
	int retval = 0;

	CimClientInfo *cimclient = NULL;

	if ( enumInfo ) {
		cimclient = CimResource_Init(cntx,
				enumInfo->auth_data.username,
				enumInfo->auth_data.password );
		if (!cimclient) {
			status->fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status->fault_detail_code = 0;
			retval = 1;
			goto cleanup;
		}
	}

	if (!verify_class_namespace(cimclient)) {
		error("resource uri namespace mismatch");
		status->fault_code = WSA_DESTINATION_UNREACHABLE;
		status->fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
		retval = 1;
		goto cleanup;
	}

	wsman_parse_enum_request(cntx, enumInfo);
	cim_enum_instances(cimclient, enumInfo, status);

	if (status && status->fault_code != 0) {
		retval = 1;
		goto cleanup;
	}
	if (enumInfo->flags & WSMAN_ENUMINFO_OPT) {
		doc = wsman_create_response_envelope( cntx->indoc , NULL);
		WsXmlNodeH node = ws_xml_add_child(ws_xml_get_soap_body(doc),
				XML_NS_ENUMERATION, WSENUM_ENUMERATE_RESP , NULL);
		cim_get_enum_items(cimclient, cntx, node,
				enumInfo, XML_NS_WS_MAN, enumInfo->maxItems,
				enumInfo->maxsize);
		int index2 = enumInfo->index + 1;
		if (enumInfo->totalItems == 0 ||index2 == enumInfo->totalItems)  {
			cim_release_enum_context(enumInfo);
			CimResource_destroy(cimclient);
			return retval;
		}
	}
cleanup:
	if (retval && cimclient) {
		CimResource_destroy(cimclient);
	}
	else if(cimclient && cimclient->selectors) {
		hash_free(cimclient->selectors);
		cimclient->selectors = NULL;
		debug("selectors destroyed");
	}
	return retval;
}


int
CimResource_Release_EP( WsContextH cntx,
		WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	debug( "Release Endpoint Called");
	if(enumInfo->flags & WSMAN_ENUMINFO_CIM_CONTEXT_CLEANUP)
		return 0;
	CimClientInfo * cimclient = cim_getclient_from_enum_context(enumInfo);
	cim_release_enum_context(enumInfo);
	if (cimclient) {
		CimResource_destroy(cimclient);
	}
	return 0;
}


int
CimResource_Pull_EP( WsContextH cntx,
		WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData)
{

	WsXmlDocH doc = NULL;
	CimClientInfo *cimclient = NULL;
	WsXmlNodeH body, pullnode;
	int maxelements;
	unsigned long maxsize;
	debug( "Pull Endpoint Called");

	if ( enumInfo) {
		cimclient = cim_getclient_from_enum_context(enumInfo);
		if (!cimclient) {
			status->fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status->fault_detail_code = 0;
			doc = wsman_generate_fault( cntx->indoc, status->fault_code,
				status->fault_detail_code, NULL);
			goto cleanup;
		}
		cimclient->cntx = cntx;
	}
	if (!cimclient)
		goto cleanup;
	if (!verify_class_namespace(cimclient) ) {
		status->fault_code = WSA_DESTINATION_UNREACHABLE;
		status->fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
		doc = wsman_generate_fault( cntx->indoc, status->fault_code,
				status->fault_detail_code, NULL);
		goto cleanup;
	}

	doc = wsman_create_response_envelope( cntx->indoc, NULL);
	body = ws_xml_get_soap_body(doc);
	pullnode = ws_xml_add_child(body, XML_NS_ENUMERATION,
			WSENUM_PULL_RESP, NULL);

	maxelements= wsman_get_max_elements(cntx, NULL);
	maxsize = wsman_get_max_envelope_size(cntx, NULL);
	if(maxsize == 0) {
		body = ws_xml_get_soap_body(cntx->indoc);
		body = ws_xml_get_child(body, 0, XML_NS_ENUMERATION, WSENUM_PULL);
		maxsize = ws_deserialize_uint32(NULL, body,
					     0, XML_NS_ENUMERATION,
					     WSENUM_MAX_CHARACTERS);
	}
	cim_get_enum_items(cimclient, cntx, pullnode,
			enumInfo, XML_NS_ENUMERATION,  maxelements, maxsize);

cleanup:
	if ( enumInfo->totalItems == 0 ||
		( enumInfo->index + 1 ) == enumInfo->totalItems) {
		cim_release_enum_context(enumInfo);
		if (cimclient) {
			CimResource_destroy(cimclient);
		}
		enumInfo->flags |= WSMAN_ENUMINFO_CIM_CONTEXT_CLEANUP;
	}

	ws_destroy_context(cntx);
	return 0;
}

int
CimResource_Create_EP( SoapOpH op,
		void* appData,
		void *opaqueData )
{

	WsXmlDocH doc = NULL;
	CimClientInfo *cimclient = NULL;
	WsmanStatus status;
	char *fragstr = NULL;

	SoapH soap = soap_get_op_soap(op);
	WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
	WsmanMessage *msg = wsman_get_msg_from_op(op);
	debug( "Create Endpoint Called");
	wsman_status_init(&status);

	if (msg) {
		cimclient = CimResource_Init(cntx,
				msg->auth_data.username, msg->auth_data.password );
		if (!cimclient) {
			status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status.fault_detail_code = 0;
			goto cleanup;
		}
	}
	if (!verify_class_namespace(cimclient) ) {
		status.fault_code = WSA_DESTINATION_UNREACHABLE;
		status.fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
		goto cleanup;
	}

	if ((doc = wsman_create_response_envelope( soap_get_op_doc(op, 1), NULL))) {
		WsXmlNodeH body = ws_xml_get_soap_body(doc);
		WsXmlNodeH in_body = ws_xml_get_soap_body(soap_get_op_doc(op, 1));
		if (!ws_xml_get_child(in_body, 0, NULL, NULL)) {
			status.fault_code = WSMAN_SCHEMA_VALIDATION_ERROR;
			status.fault_detail_code = 0;
		} else {
			char *xsd = u_strdup_printf("%s.xsd", cimclient->resource_uri);
			fragstr = wsman_get_fragment_string(cntx, cntx->indoc);
			if(fragstr) {
				if(ws_xml_get_child(in_body, 0, XML_NS_WS_MAN, WSM_XML_FRAGMENT))
					cim_create_instance(cimclient, cntx, in_body, body, fragstr, &status);
			}
			else {
				if (ws_xml_get_child(in_body, 0, cimclient->resource_uri, cimclient->requested_class)) {
					cim_create_instance(cimclient, cntx , in_body, body, NULL, &status);
				} else if (ws_xml_get_child(in_body, 0, xsd, cimclient->requested_class)) {
					cim_create_instance(cimclient, cntx , in_body, body, NULL, &status);
				} else {
					status.fault_code = WXF_INVALID_REPRESENTATION;
					status.fault_detail_code = WSMAN_DETAIL_INVALID_NAMESPACE;
				}
			}
			u_free(xsd);
		}
	}

cleanup:
	if (wsman_check_status(&status) != 0) {
		ws_xml_destroy_doc(doc);
		doc = wsman_generate_fault( soap_get_op_doc(op, 1),
				status.fault_code, status.fault_detail_code, NULL);
	}

	if ( doc ) {
		soap_set_op_doc(op, doc, 0);
	} else {
		debug( "Invalid doc" );
	}

	CimResource_destroy(cimclient);
	ws_destroy_context(cntx);
	return 0;
}



int
CimResource_Put_EP( SoapOpH op,
		void* appData,
		void *opaqueData )
{
	debug( "Put Endpoint Called");
	WsXmlDocH doc = NULL;
	CimClientInfo *cimclient = NULL;
	WsmanStatus status;
	WsmanMessage *msg;
	char *fragstr;

	SoapH soap = soap_get_op_soap(op);
	WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
	WsXmlDocH indoc = soap_get_op_doc(op, 1);

	wsman_status_init(&status);
	msg = wsman_get_msg_from_op(op);

	if (msg) {
		cimclient = CimResource_Init(cntx,
				msg->auth_data.username, msg->auth_data.password );
		if (!cimclient) {
			status.fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status.fault_detail_code = 0;
			goto cleanup;
		}
	} else {
		goto cleanup;
	}

	if (!verify_class_namespace(cimclient) ) {
		status.fault_code = WSA_DESTINATION_UNREACHABLE;
		status.fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
	}

	if ((doc = wsman_create_response_envelope( indoc, NULL))) {
		WsXmlNodeH body = ws_xml_get_soap_body(doc);
		WsXmlNodeH in_body = ws_xml_get_soap_body(indoc);
		fragstr = wsman_get_fragment_string(cntx, indoc);
		if(fragstr)
			body = ws_xml_add_child(body, XML_NS_WS_MAN, WSM_XML_FRAGMENT,
			NULL);
		if (ws_xml_get_child(in_body, 0, NULL, NULL)) {
			cim_put_instance(cimclient, cntx , in_body, body, fragstr, &status);
		} else {
			status.fault_code = WXF_INVALID_REPRESENTATION;
			status.fault_detail_code = WSMAN_DETAIL_MISSING_VALUES;
		}
	}

cleanup:
	if (wsman_check_status(&status) != 0) {
		ws_xml_destroy_doc(doc);
		doc = wsman_generate_fault( indoc,
				status.fault_code, status.fault_detail_code, NULL);
	}

	if ( doc ) {
		soap_set_op_doc(op, doc, 0);
	} else {
		debug( "Invalid doc" );
	}

	CimResource_destroy(cimclient);
	ws_destroy_context(cntx);
	return 0;
}
#ifdef ENABLE_EVENTING_SUPPORT
int
CimResource_EventPoll_EP(WsEventThreadContextH cntx)
{
	int retval = 0;
	return retval;
}

int
CimResource_Subscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	debug("CIM Subscription");
	int retval = 0;
	CimClientInfo *cimclient = NULL;
	CMPIInstance *instance = NULL;
	CMPIObjectPath *indicationfilter = NULL;
	CMPIObjectPath *indicationhandler = NULL;
	CMPIObjectPath *indicationsubscription = NULL;
	if ( subsInfo ) {
		cimclient = CimResource_Init(cntx,
				subsInfo->auth_data.username,
				subsInfo->auth_data.password );
		if (!cimclient) {
			status->fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status->fault_detail_code = 0;
			retval = 1;
			goto cleanup;
		}
	}

	if (!verify_class_namespace(cimclient)) {
		error("resource uri namespace mismatch");
		status->fault_code = WSA_DESTINATION_UNREACHABLE;
		status->fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
		retval = 1;
		goto cleanup;
	}
// to do here: create indication filter here and something else necessary
	subsInfo->eventpoll= CimResource_EventPoll_EP;
	subsInfo->cancel = CimResource_SubscriptionCancel_EP;
	subsInfo->vendor_namespaces = cimclient->namespaces;
	subsInfo->cim_namespace = u_strdup(cimclient->cim_namespace);
	if(subsInfo->flags & WSMAN_SUBSCRIPTION_SELECTORSET) { //Subscribe to an Indication filter instance
		indicationfilter= cim_get_objectpath_from_selectors(cimclient, cntx, status);
		if(indicationfilter)
			subsInfo->existingfilterOP = CMClone(indicationfilter, NULL);
		else {
			status->fault_code = WSA_DESTINATION_UNREACHABLE;
			status->fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
		}

		debug("subscribe to an existing filter");
	}
	else {
		indicationfilter = cim_create_indication_filter(cimclient, subsInfo, status);
	}
	if(status->fault_code ) {
		retval = 1;
		goto cleanup;
	}

	indicationhandler = cim_create_indication_handler(cimclient, subsInfo, status);
	if(status->fault_code) {
		retval = 1;
		goto cleanup;
	}
	cim_create_indication_subscription(cimclient, subsInfo,
		indicationfilter, indicationhandler, status);
	if(status->fault_code)
		retval = 1;
cleanup:
	if(instance)
		CMRelease(instance);
	if(indicationfilter)
		CMRelease(indicationfilter);
	if(indicationhandler)
		CMRelease(indicationhandler);
	if(indicationsubscription)
		CMRelease(indicationsubscription);
	CimResource_destroy(cimclient);
	return retval;
}

int CimResource_Renew_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	debug("CIM Renew");
	int retval = 0;
	CimClientInfo *cimclient = NULL;
	if ( subsInfo ) {
		cimclient = CimResource_Init(cntx,
				subsInfo->auth_data.username,
				subsInfo->auth_data.password );
		if (!cimclient) {
			status->fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status->fault_detail_code = 0;
			retval = 1;
			return retval;
		}
	}
	cim_update_indication_subscription(cimclient, subsInfo, status);
	if(status->fault_code)
		retval = 1;
	CimResource_destroy(cimclient);
	return retval;
}

int CimResource_UnSubscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	debug("CIM UnSubscribe");
	int retval = 0;
	CimClientInfo *cimclient = NULL;
	if ( subsInfo ) {
		cimclient = CimResource_Init(cntx,
				subsInfo->auth_data.username,
				subsInfo->auth_data.password );
		if (!cimclient) {
			status->fault_code = WSA_ENDPOINT_UNAVAILABLE;
			status->fault_detail_code = 0;
			retval = 1;
			return retval;
		}
	}
	cim_delete_indication_subscription(cimclient, subsInfo, status);
	if(status->fault_code)
		retval = 1;
	CimResource_destroy(cimclient);
	return retval;
}

int CimResource_SubscriptionCancel_EP(WsEventThreadContextH cntx)
{
	WsmanStatus status;
	return CimResource_UnSubscribe_EP(cntx->soap->cntx, cntx->subsInfo, &status, NULL);
}
#endif
