/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,cl
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
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGclE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Anas Nashif
 */

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-dispatcher.h"
#include "wsman-soap-envelope.h"

#include "wsman-xml.h"
#include "wsman-xml-serialize.h"
#include "wsman-client-transport.h"
#include "wsman-faults.h"
#include "wsman-client.h"

static hash_t *
get_selectors_from_uri(const char *resource_uri)
{
	u_uri_t        *uri;
	hash_t *selectors = NULL;
	if (resource_uri != NULL) {
		if (u_uri_parse((const char *) resource_uri, &uri) != 0)
			return NULL;
	} else {
		return NULL;
	}
	if (uri->query != NULL) {
		 selectors = u_parse_query(uri->query);
	}
	if (uri) {
		u_uri_free(uri);
	}
	return selectors;
}
void
wsman_client_set_dumpfile( WsManClient *cl, FILE *f )
{
    if (f)
	cl->dumpfile = f;
    return;
}

FILE *
wsman_client_get_dumpfile(WsManClient *cl)
{
    return cl->dumpfile;
}


static char*
wsman_make_action(char *uri, char *op_name)
{
	size_t     len = strlen(uri) + strlen(op_name) + 2;
	char    *ptr = (char *) malloc(len);
	if (ptr) {
		sprintf(ptr, "%s/%s", uri, op_name);
	}
	return ptr;
}



WsContextH
wsman_create_runtime (void)
{
	return ws_create_runtime(NULL);
}




// Access to client elements
WS_LASTERR_Code
wsman_client_get_last_error(WsManClient * cl)
{
	return cl->last_error;
}


long
wsman_client_get_response_code(WsManClient * cl)
{
	return cl->response_code;
}

char*
wsman_client_get_fault_string(WsManClient * cl)
{
	return cl->fault_string;
}

WsContextH
wsman_client_get_context(WsManClient * cl)
{
	return cl->wscntx;
}


char *
wsman_client_get_hostname(WsManClient * cl)
{
	return cl->data.hostname;
}


unsigned int
wsman_client_get_port(WsManClient * cl)
{
	return cl->data.port;
}

char *
wsman_client_get_scheme(WsManClient * cl)
{
	return cl->data.scheme;
}


char *
wsman_client_get_path(WsManClient * cl)
{
	return cl->data.path;
}


char *
wsman_client_get_user(WsManClient * cl)
{
	return cl->data.user;
}


char *
wsman_client_get_endpoint(WsManClient * cl)
{
	return cl->data.endpoint;
}




WsXmlDocH
wsman_client_read_file(WsManClient * cl, char *filename,
		char *encoding, unsigned long options)
{
	return ws_xml_read_file(ws_context_get_runtime(cl->wscntx),
			filename, encoding, options);
}

WsXmlDocH
wsman_client_read_memory(WsManClient * cl, char *buf,
		size_t size, char *encoding, unsigned long options)
{
	return ws_xml_read_memory(ws_context_get_runtime(cl->wscntx),
			buf, size, encoding, options);
}

actionOptions *
initialize_action_options(void)
{
	actionOptions *op = u_malloc(sizeof(actionOptions));
	if (op)
		memset(op, 0, sizeof(actionOptions));
	else 
		return NULL;
	return op;
}


void
destroy_action_options(actionOptions * op)
{
	if (op->selectors) {
		hash_free(op->selectors);
	}
	if (op->properties) {
		hash_free(op->properties);
	}
	u_free(op);
	return;
}

void
wsman_set_action_option(actionOptions * options, unsigned int flag)
{
	options->flags |= flag;
	return;
}


void
wsman_client_add_property(actionOptions * options,
		const char *key,
		const char *value)
{
	if (options->properties == NULL)
		options->properties = hash_create(HASHCOUNT_T_MAX, 0, 0);
	if (!hash_lookup(options->properties, key)) {
		if (!hash_alloc_insert(options->properties,
					(char *)key, (char *)value)) {
			error("hash_alloc_insert failed");
		}
	} else {
		error("duplicate not added to hash");
	}
}

void
wsman_client_add_selector(actionOptions * options,
		const char *key,
		const char *value)
{
	if (options->selectors == NULL)
		options->selectors = hash_create(HASHCOUNT_T_MAX, 0, 0);
	if (!hash_lookup(options->selectors, key)) {
		if (!hash_alloc_insert(options->selectors,
					(char *)key, (char *)value)) {
			error( "hash_alloc_insert failed");
		}
	} else {
		error( "duplicate not added to hash");
	}
}


void
wsman_add_selectors_from_query_string(actionOptions * options,
		const char *query_string)
{
	if (query_string) {
		hash_t *query = u_parse_query(query_string);
		if (query) {
			options->selectors = query;
		}
	}
}

void
wsman_add_properties_from_query_string(actionOptions * options,
		const char *query_string)
{
	hash_t *query;
	if (!query_string) 
		return;

        query = u_parse_query(query_string);
	if (query) {
		options->properties = query;
	}
}

void
wsman_add_selector_from_options(WsXmlDocH doc, actionOptions *options)
{
	WsXmlNodeH      header;
	hnode_t        *hn;
	hscan_t         hs;
	if (!options->selectors || hash_count(options->selectors) == 0)
		return;
	header = ws_xml_get_soap_header(doc);
	hash_scan_begin(&hs, options->selectors);
	while ((hn = hash_scan_next(&hs))) {
		wsman_add_selector(header,
				(char *) hnode_getkey(hn), (char *) hnode_get(hn));
		debug("key = %s value=%s",
				(char *) hnode_getkey(hn), (char *) hnode_get(hn));
	}
}



static
void wsman_create_epr(WsContextH cntx, WsXmlNodeH epr_node, 
		const char* resource_uri, 
		actionOptions *options) 
{
	WsXmlNodeH node;
	hash_t *selectors = NULL;
	char *uri = NULL;

	ws_xml_add_child(epr_node, XML_NS_ADDRESSING, WSA_ADDRESS, 
			WSA_TO_ANONYMOUS);
	node = ws_xml_add_child(epr_node, XML_NS_ADDRESSING, 
			WSA_REFERENCE_PARAMETERS, NULL);
	if (options->selectors != NULL && 
			hash_count(options->selectors) > 0) {
		selectors = options->selectors;
		ws_serialize_str(cntx, node, resource_uri, XML_NS_WS_MAN, 
			WSM_RESOURCE_URI, 0);
	} else if (options->filter &&
		       	strcmp(options->dialect, WSM_ASSOCIATION_FILTER_DIALECT) == 0) {
		selectors =  get_selectors_from_uri(options->filter);
		wsman_remove_query_string(options->filter, &uri);
		ws_serialize_str(cntx, node, uri, XML_NS_WS_MAN, 
			WSM_RESOURCE_URI, 0);
	}

	if (selectors) {
		hnode_t        *hn;
		hscan_t         hs;
		hash_scan_begin(&hs, selectors);
		while ((hn = hash_scan_next(&hs))) {
			wsman_add_selector(node,
					(char *) hnode_getkey(hn), (char *) hnode_get(hn));
			debug("key = %s value=%s",
					(char *) hnode_getkey(hn), (char *) hnode_get(hn));
		}
	}
	if (options->cim_ns) {
		wsman_add_selector(node, CIM_NAMESPACE_SELECTOR, options->cim_ns);
	}
	return;
}



void
wsman_build_assocRef_body(WsManClient *cl, WsXmlNodeH node,
		const char *resource_uri,
		actionOptions *options,
		int assocRef)
{
	WsXmlNodeH  object, assInst;

	if((options->flags & FLAG_CIM_REFERENCES) == FLAG_CIM_REFERENCES)
		assInst = ws_xml_add_child(node, XML_NS_CIM_BINDING,
				WSMB_ASSOCIATION_INSTANCES, NULL);
	else if((options->flags & FLAG_CIM_ASSOCIATORS) == FLAG_CIM_ASSOCIATORS)
		assInst = ws_xml_add_child(node, XML_NS_CIM_BINDING,
				WSMB_ASSOCIATED_INSTANCES, NULL);
	else
		return;

	/* Build Object */
	object = ws_xml_add_child(assInst, XML_NS_CIM_BINDING, WSMB_OBJECT, NULL);

	wsman_create_epr(cl->wscntx, object, resource_uri, options );
	/* Add AssociationClassName */
	node = ws_xml_add_child(assInst, XML_NS_CIM_BINDING, 
			WSMB_ASSOCIATION_CLASS_NAME, NULL);
	/* Add ResultClassName */
	node = ws_xml_add_child(assInst, XML_NS_CIM_BINDING, 
			WSMB_RESULT_CLASS_NAME, NULL);
	/* Add Role */
	node = ws_xml_add_child(assInst, XML_NS_CIM_BINDING, 
			WSMB_ROLE, NULL);
	if((options->flags & FLAG_CIM_ASSOCIATORS) == FLAG_CIM_ASSOCIATORS) {
		/* Add Role */
		node = ws_xml_add_child(assInst, XML_NS_CIM_BINDING, 
				WSMB_RESULT_ROLE, NULL);
	}
	/* Add IncludeResultProperty */
	ws_xml_add_child(assInst, XML_NS_CIM_BINDING,
		       	WSMB_INCLUDE_RESULT_PROPERTY, NULL);

}

void
wsman_set_options_from_uri(const char *resource_uri, actionOptions * options)
{
	options->selectors = get_selectors_from_uri(resource_uri);
}

void
wsman_add_selector_from_uri(WsXmlDocH doc,
		const char *resource_uri)
{
	u_uri_t        *uri;
	WsXmlNodeH      header = ws_xml_get_soap_header(doc);
	hash_t         *query;
	hnode_t        *hn;
	hscan_t         hs;

	if (resource_uri != NULL) {
		if (u_uri_parse((const char *) resource_uri, &uri) != 0)
			return;
		else if (!uri->query)
			goto cleanup;
	}

	query = u_parse_query(uri->query);
	hash_scan_begin(&hs, query);
	while ((hn = hash_scan_next(&hs))) {
		wsman_add_selector(header,
				(char *) hnode_getkey(hn),
				(char *) hnode_get(hn));
		debug("key=%s value=%s", (char *) hnode_getkey(hn),
				(char *) hnode_get(hn));
	}
	hash_free_nodes(query);
	hash_destroy(query);
cleanup:
	if (uri) {
		u_uri_free(uri);
	}
}


static char*
wsman_create_action_str(WsmanAction action)
{
	char           *action_str = NULL;

	switch (action) {
	case WSMAN_ACTION_ENUMERATION:
		action_str = wsman_make_action(XML_NS_ENUMERATION, WSENUM_ENUMERATE);
		break;
	case WSMAN_ACTION_PULL:
		action_str = wsman_make_action(XML_NS_ENUMERATION, WSENUM_PULL);
		break;
	case WSMAN_ACTION_RELEASE:
		action_str = wsman_make_action(XML_NS_ENUMERATION, WSENUM_RELEASE);
		break;
	case WSMAN_ACTION_TRANSFER_CREATE:
		action_str = wsman_make_action(XML_NS_TRANSFER, TRANSFER_CREATE);
		break;
	case WSMAN_ACTION_TRANSFER_DELETE:
		action_str = wsman_make_action(XML_NS_TRANSFER, TRANSFER_DELETE);
		break;
	case WSMAN_ACTION_TRANSFER_GET:
		action_str = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);
		break;
	case WSMAN_ACTION_TRANSFER_PUT:
		action_str = wsman_make_action(XML_NS_TRANSFER, TRANSFER_PUT);
		break;
	case WSMAN_ACTION_NONE:
	case WSMAN_ACTION_IDENTIFY:
	case WSMAN_ACTION_TEST:
	case WSMAN_ACTION_CUSTOM:
		break;
	}
	return action_str;
}




static void
wsman_set_enumeration_options(WsManClient * cl,
			WsXmlNodeH body, 
			const char* resource_uri,
			actionOptions *options)
{
	WsXmlNodeH filter = NULL;
	WsXmlNodeH      node = ws_xml_get_child(body, 0, NULL, NULL);
	if ((options->flags & FLAG_ENUMERATION_OPTIMIZATION) ==
			FLAG_ENUMERATION_OPTIMIZATION) {
		ws_xml_add_child(node, XML_NS_WS_MAN, WSM_OPTIMIZE_ENUM, NULL);
	}

	if ((options->flags & FLAG_ENUMERATION_ENUM_EPR) ==
			FLAG_ENUMERATION_ENUM_EPR) {
		ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE, WSM_ENUM_EPR);
	} else if ((options->flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) ==
			FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) {
		ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE,
				WSM_ENUM_OBJ_AND_EPR);
	}

	// Polymorphism
	if ((options->flags & FLAG_IncludeSubClassProperties) 
			== FLAG_IncludeSubClassProperties) {
		ws_xml_add_child(node, XML_NS_CIM_BINDING,
				WSMB_POLYMORPHISM_MODE, WSMB_INCLUDE_SUBCLASS_PROP);
	} else if ((options->flags & FLAG_ExcludeSubClassProperties) == 
			FLAG_ExcludeSubClassProperties) {
		ws_xml_add_child(node, XML_NS_CIM_BINDING,
				WSMB_POLYMORPHISM_MODE, WSMB_EXCLUDE_SUBCLASS_PROP);
	} else if ((options->flags & FLAG_POLYMORPHISM_NONE) 
			== FLAG_POLYMORPHISM_NONE) {
		ws_xml_add_child(node, XML_NS_CIM_BINDING,
				WSMB_POLYMORPHISM_MODE, "None");
	}
	if (options->filter) {
		if (((options->flags & FLAG_CIM_REFERENCES) == FLAG_CIM_REFERENCES) ||
			((options->flags & FLAG_CIM_ASSOCIATORS) == FLAG_CIM_ASSOCIATORS)) {
			filter = ws_xml_add_child(node,
				XML_NS_WS_MAN, WSENUM_FILTER, NULL);
		} else {
			filter = ws_xml_add_child(node,
				XML_NS_WS_MAN, WSENUM_FILTER, options->filter);
		}
		if (options->dialect)
			ws_xml_add_node_attr(filter, NULL, WSENUM_DIALECT, options->dialect);
		else
			ws_xml_add_node_attr(filter, NULL, WSENUM_DIALECT, WSM_XPATH_FILTER_DIALECT );
	}

	// References and Associations
	if (((options->flags & FLAG_CIM_REFERENCES) == FLAG_CIM_REFERENCES) ||
			((options->flags & FLAG_CIM_ASSOCIATORS) == FLAG_CIM_ASSOCIATORS)) {
		wsman_build_assocRef_body(cl, filter, resource_uri, options, 0);
	}
}

static void
wsman_set_transfer_put_properties(WsXmlDocH get_response,
		WsXmlDocH put_request,
		actionOptions *options)
{
	WsXmlNodeH      resource_node;
	char           *ns_uri;
	hscan_t         hs;
	hnode_t        *hn;
	WsXmlNodeH      get_body = ws_xml_get_soap_body(get_response);
	WsXmlNodeH      put_body = ws_xml_get_soap_body(put_request);

	ws_xml_copy_node(ws_xml_get_child(get_body, 0, NULL, NULL), put_body);
	resource_node = ws_xml_get_child(put_body, 0, NULL, NULL);
	ns_uri = ws_xml_get_node_name_ns_uri(resource_node);

	if (!options->properties) {
		return;
	}
	hash_scan_begin(&hs, options->properties);
	while ((hn = hash_scan_next(&hs))) {
		WsXmlNodeH      n = ws_xml_get_child(resource_node, 0,
				ns_uri, (char *) hnode_getkey(hn));
		ws_xml_set_node_text(n, (char *) hnode_get(hn));
	}
}


void
wsman_client_node_to_buf(WsXmlNodeH node, char **buf) {
	int   len;
	WsXmlDocH doc = ws_xml_create_doc_by_import( node);
	ws_xml_dump_memory_enc(doc, buf, &len, "UTF-8");
	ws_xml_destroy_doc(doc);
	return;
}


char*
wsman_client_node_to_formatbuf(WsXmlNodeH node) {
	char *buf;
	int   len;
	WsXmlDocH doc = ws_xml_create_doc_by_import( node);
	ws_xml_dump_memory_node_tree(ws_xml_get_doc_root(doc), &buf, &len);
	ws_xml_destroy_doc(doc);
	return buf;
}


WsXmlDocH
wsman_client_create_request(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		WsmanAction action,
		char *method,
		void *data)
{
	WsXmlDocH       request;
	WsXmlNodeH      body;
	WsXmlNodeH      header;
	WsXmlNodeH      node;
	char           *_action = NULL;

	if (action == WSMAN_ACTION_IDENTIFY) {
		request = ws_xml_create_envelope(
				ws_context_get_runtime(cl->wscntx), NULL);
	} else {
		if (method) {
			if (strchr(method, '/'))
				_action = u_strdup(method);
			else
				_action = wsman_make_action((char *)resource_uri, method);
		} else {
			_action = wsman_create_action_str(action);
		}

		request = wsman_build_envelope(cl->wscntx, _action,
				WSA_TO_ANONYMOUS, (char *)resource_uri,
				cl->data.endpoint, options);
		u_free(_action);
	}

	body = ws_xml_get_soap_body(request);
	header = ws_xml_get_soap_header(request);
	if ((options->flags & FLAG_CIM_EXTENSIONS) == FLAG_CIM_EXTENSIONS) {
		WsXmlNodeH opset = ws_xml_add_child(header, 
				XML_NS_WS_MAN, WSM_OPTION_SET, NULL);
		WsXmlNodeH op = ws_xml_add_child(opset, 
				XML_NS_WS_MAN, WSM_OPTION, NULL);
		ws_xml_add_node_attr(op, NULL, WSM_NAME, WSMB_SHOW_EXTENSION);
	}


	switch (action) {
	case WSMAN_ACTION_IDENTIFY:
		ws_xml_add_child(ws_xml_get_soap_body(request),
				XML_NS_WSMAN_ID, WSMID_IDENTIFY, NULL);
		break;
        case WSMAN_ACTION_CUSTOM:
                break;
        case WSMAN_ACTION_ENUMERATION:
		node = ws_xml_add_child(ws_xml_get_soap_body(request),
				XML_NS_ENUMERATION, WSENUM_ENUMERATE, NULL);
		wsman_set_enumeration_options(cl, body, resource_uri, options);
		break;
	case WSMAN_ACTION_PULL:
		node = ws_xml_add_child(ws_xml_get_soap_body(request),
				XML_NS_ENUMERATION, WSENUM_PULL, NULL);
		if (data) {
			ws_xml_add_child(node, XML_NS_ENUMERATION,
					WSENUM_ENUMERATION_CONTEXT, (char *) data);
		}
		break;
	case WSMAN_ACTION_RELEASE:
		node = ws_xml_add_child(ws_xml_get_soap_body(request),
				XML_NS_ENUMERATION, WSENUM_RELEASE, NULL);
		if (data) {
			ws_xml_add_child(node, XML_NS_ENUMERATION,
					WSENUM_ENUMERATION_CONTEXT, (char *) data);
		}
		break;
	case WSMAN_ACTION_NONE:
	case WSMAN_ACTION_TRANSFER_CREATE:  
	case WSMAN_ACTION_TEST:
	case WSMAN_ACTION_TRANSFER_GET:
	case WSMAN_ACTION_TRANSFER_PUT:
	case WSMAN_ACTION_TRANSFER_DELETE:
		break;
	}

	if (action == WSMAN_ACTION_PULL || action == WSMAN_ACTION_ENUMERATION) {
		if (options->max_elements > 0 ) {
			node = ws_xml_get_child(body, 0, NULL, NULL);
			if ((options->flags & FLAG_ENUMERATION_OPTIMIZATION) ==
					FLAG_ENUMERATION_OPTIMIZATION ) {
				ws_xml_add_child_format(node, XML_NS_WS_MAN,
						WSENUM_MAX_ELEMENTS, "%d", options->max_elements);
			} else {
				ws_xml_add_child_format(node, XML_NS_ENUMERATION,
						WSENUM_MAX_ELEMENTS, "%d", options->max_elements);
			}
		}
		if ((options->flags & FLAG_ENUMERATION_COUNT_ESTIMATION) ==
				FLAG_ENUMERATION_COUNT_ESTIMATION) {
			ws_xml_add_child(header, XML_NS_WS_MAN, WSM_REQUEST_TOTAL, NULL);
		}
	}
	if (action != WSMAN_ACTION_TRANSFER_CREATE &&
			action != WSMAN_ACTION_TRANSFER_PUT &&
			action != WSMAN_ACTION_CUSTOM) {
		if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
			ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
		}
	}
	return request;
}



static void
handle_resource_request(WsManClient * cl, WsXmlDocH request, 
		void *data, 
		void *typeInfo,
		char *resource_uri)
{
	if (data && typeInfo) {
		char           *class = u_strdup(strrchr(resource_uri, '/') + 1);
		ws_serialize(cl->wscntx, ws_xml_get_soap_body(request),
				data, (XmlSerializerInfo *) typeInfo,
				class, resource_uri, NULL, 1);
		ws_serializer_free_mem(cl->wscntx, data,
				(XmlSerializerInfo *) typeInfo);
		u_free(class);
	} else if (data != NULL) {
		if (wsman_is_valid_xml_envelope((WsXmlDocH) data)) {
			WsXmlNodeH      body = ws_xml_get_soap_body((WsXmlDocH) data);
			ws_xml_duplicate_tree(ws_xml_get_soap_body(request),
					ws_xml_get_child(body, 0, NULL, NULL));
		} else {
			ws_xml_duplicate_tree(ws_xml_get_soap_body(request),
					ws_xml_get_doc_root((WsXmlDocH) data));
		}
	}	
}


static          WsXmlDocH
_ws_transfer_create(WsManClient * cl,
		char *resource_uri,
		void *data,
		void *typeInfo,
		actionOptions *options)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsman_client_create_request(cl,
			(char *)resource_uri, options,
			WSMAN_ACTION_TRANSFER_CREATE, NULL, NULL);
	handle_resource_request(cl, request, data, typeInfo, (char *)resource_uri);

	if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
		ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
	}
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}

WsXmlDocH
ws_transfer_create(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		WsXmlDocH source_doc)
{

	return _ws_transfer_create(cl, (char *)resource_uri,
			source_doc, NULL, options);
}


WsXmlDocH
ws_transfer_create_fromtext(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		const char *data, size_t size, const char *encoding)
{
	WsXmlDocH source_doc = wsman_client_read_memory(cl, (char *)data, size,
			(char *)encoding, 0);
	WsXmlDocH response;
	if (source_doc == NULL) {
		error("could not convert XML text to doc");
		return NULL;
	}

	response = _ws_transfer_create(cl, (char *)resource_uri,
			source_doc, NULL, options);
	ws_xml_destroy_doc(source_doc);
	return response;
}

WsXmlDocH
ws_transfer_create_serialized(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		void *data, 
		void *typeInfo) 
{
	return _ws_transfer_create(cl, (char *)resource_uri, data, typeInfo, options);	
}


static WsXmlDocH
_ws_transfer_put(WsManClient * cl,
		char *resource_uri,
		void *data,
		void *typeInfo,
		actionOptions *options)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsman_client_create_request(cl,
			resource_uri, options,
			WSMAN_ACTION_TRANSFER_PUT, NULL, NULL);
	handle_resource_request(cl, request, data, typeInfo, resource_uri);
	if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
		ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
	}
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}

WsXmlDocH
ws_transfer_put(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		WsXmlDocH source_doc)
{
	return _ws_transfer_put(cl, (char *)resource_uri, source_doc, NULL, options);
}


WsXmlDocH
ws_transfer_put_fromtext(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		const char *data, size_t size, const char *encoding)
{
	WsXmlDocH source_doc = wsman_client_read_memory(cl, (char *)data, size,
			(char *)encoding, 0);
	WsXmlDocH response;
	if (source_doc == NULL) {
		error("could not convert XML text to doc");
		return NULL;
	}
	response =  _ws_transfer_put(cl, (char *)resource_uri, source_doc, NULL, options);
	ws_xml_destroy_doc(source_doc);
	return response;
}


WsXmlDocH
ws_transfer_put_serialized(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		void *data,
		void *typeInfo)
{
	return _ws_transfer_put(cl, (char *)resource_uri, data, typeInfo, options);
}

WsXmlDocH
ws_transfer_delete(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options)
{

	WsXmlDocH       response;
	WsXmlDocH       request = wsman_client_create_request(cl,
			resource_uri, options,
			WSMAN_ACTION_TRANSFER_DELETE, NULL, NULL);
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
ws_transfer_get(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsman_client_create_request(cl,
			resource_uri, options,
			WSMAN_ACTION_TRANSFER_GET, NULL, NULL);
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
ws_transfer_get_and_put(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options)
{
	WsXmlDocH       put_request;
	WsXmlDocH       put_response;
	WsXmlDocH       get_response = ws_transfer_get(cl, resource_uri, options);

	if (!get_response) {
		error("ws_transfer_get returned NULL doc");
		return NULL;
	}
	if (wsman_is_fault_envelope(get_response)) {
		return get_response;
	}
	put_request = wsman_client_create_request(cl,
			resource_uri, options,
			WSMAN_ACTION_TRANSFER_PUT, NULL, (void *) get_response);

	wsman_set_transfer_put_properties(get_response, put_request, options);
	//ws_xml_destroy_doc(get_response);
	if (wsman_send_request(cl, put_request)) {
		ws_xml_destroy_doc(put_request);
		return NULL;
	}
	put_response = wsman_build_envelope_from_response(cl);

	//ws_xml_destroy_doc(put_request);
	return put_response;
}




WsXmlDocH
wsman_invoke(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		const char *method,
		WsXmlDocH data)
{
	hscan_t         hs;
	hnode_t        *hn;
	WsXmlDocH       response;
        WsXmlDocH       request = wsman_client_create_request(cl,
                resource_uri, options,
                WSMAN_ACTION_CUSTOM, (char *)method, NULL);
	WsXmlNodeH body = ws_xml_get_soap_body(request);

	if ((!options->properties || 
                    hash_count(options->properties) == 0) && 
                data != NULL) {

		WsXmlNodeH n = ws_xml_get_doc_root(data);
		ws_xml_duplicate_tree(ws_xml_get_soap_body(request), n);
        } else if (options->properties && 
                hash_count(options->properties) > 0 ) {
            if (method) {
                WsXmlNodeH node = ws_xml_add_empty_child_format(body,
                        (char *)resource_uri, "%s_INPUT", method);
                hash_scan_begin(&hs, options->properties);
                while ((hn = hash_scan_next(&hs))) {
                    ws_xml_add_child(node,  
                            (char *)resource_uri, 
                            (char *) hnode_getkey(hn),
                            (char *) hnode_get(hn));
                }
            }
        } else {
            ws_xml_add_empty_child_format(body,
                    (char *)resource_uri, "%s_INPUT", method);
        }
	if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
		ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
	}

	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
wsman_invoke_fromtext(WsManClient * cl,
		const char *resourceUri,
		actionOptions *options,
		const char *method,
		const char *data, 
                size_t size, 
                const char *encoding)
{
	WsXmlDocH       response;
        WsXmlDocH request = wsman_client_create_request(cl, resourceUri, options,
						  WSMAN_ACTION_CUSTOM, 
						  (char *)method, NULL);
	if (request == NULL) {
		error("could not create request");
		return NULL;
	}
	if ( data != NULL) {
		WsXmlDocH doc = wsman_client_read_memory(cl,
				(char *)data, size, (char *)encoding, 0);
		WsXmlNodeH n;
		if (doc == NULL) {
			error("could not wsman_client_read_memory");
			ws_xml_destroy_doc(request);
			return NULL;
		}
		n = ws_xml_get_doc_root(doc);
		ws_xml_duplicate_tree(ws_xml_get_soap_body(request), n);
		ws_xml_destroy_doc(doc);
	} else {
            warning("No XML provided");
        }

	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}



WsXmlDocH
wsman_invoke_serialized(WsManClient * cl,
		const char *resourceUri,
		actionOptions *options,
		const char *method,
		void *typeInfo, void *data)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsman_client_create_request(cl,
			resourceUri, options,
			WSMAN_ACTION_CUSTOM, (char *)method, NULL);
	if (request == NULL) {
		error("could not create request");
		return NULL;
	}
	if (data != NULL) {
		handle_resource_request(cl, request, data, typeInfo, (char *)resourceUri);
	}

	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
wsman_identify(WsManClient * cl,
		actionOptions *options)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsman_client_create_request(cl, NULL, options,
			WSMAN_ACTION_IDENTIFY, NULL, NULL);
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


int
wsenum_enumerate_and_pull(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		SoapResponseCallback callback,
		void *callback_data)
{
	WsXmlDocH       doc;
	char           *enumContext;
	WsXmlDocH       enum_response = wsenum_enumerate(cl,
			resource_uri, options);

	if (enum_response) {
		long rc = wsman_client_get_response_code(cl);
		if (rc == 200 || rc == 400 || rc == 500) {
			callback(cl, enum_response, callback_data);
		} else {
			return 0;
		}
		enumContext = wsenum_get_enum_context(enum_response);
		ws_xml_destroy_doc(enum_response);
	} else {
		return 0;
	}

	while (enumContext != NULL && enumContext[0] != 0) {
		long rc = wsman_client_get_response_code(cl);
		doc = wsenum_pull(cl, resource_uri, options, enumContext);

		if (rc != 200 && rc != 400 && rc != 500) {
			return 0;
		}
		callback(cl, doc, callback_data);
		enumContext = wsenum_get_enum_context(doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
	}
	return 1;
}

WsXmlDocH
wsenum_enumerate(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsman_client_create_request(cl,
			resource_uri, options,
			WSMAN_ACTION_ENUMERATION, NULL, NULL);
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsman_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
wsenum_pull(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		const char *enumContext)
{
	WsXmlDocH       response;
	WsXmlNodeH      node;

	if (enumContext || (enumContext && enumContext[0] == 0)) {
		WsXmlDocH       request = wsman_client_create_request(cl,
				resource_uri, options,
				WSMAN_ACTION_PULL,
				NULL, (char *)enumContext);
		if (wsman_send_request(cl, request)) {
			ws_xml_destroy_doc(request);
			//            u_free(enumContext);
			return NULL;
		}
		response = wsman_build_envelope_from_response(cl);
		//        u_free(enumContext);
		ws_xml_destroy_doc(request);
	} else {
		error("No enumeration context ???");
		return NULL;
	}

	node = ws_xml_get_child(ws_xml_get_soap_body(response),
			0, NULL, NULL);

	if (node == NULL ||
			(strcmp(ws_xml_get_node_local_name(node), WSENUM_PULL_RESP)) != 0) {
		error("no Pull response");
	}
	return response;
}


WsXmlDocH
wsenum_release(WsManClient * cl,
		const char *resource_uri,
		actionOptions *options,
		const char *enumContext)
{
	WsXmlDocH       response;

	if (enumContext || (enumContext && enumContext[0] == 0)) {
		WsXmlDocH       request = wsman_client_create_request(cl,
				resource_uri, options,
				WSMAN_ACTION_RELEASE,
				NULL, (char *)enumContext);
		if (wsman_send_request(cl, request)) {
			ws_xml_destroy_doc(request);
			//            u_free(enumContext);
			return NULL;
		}
		response = wsman_build_envelope_from_response(cl);
		//        u_free(enumContext);
		ws_xml_destroy_doc(request);
	} else {
		return NULL;
	}
	return response;
}

char*
wsenum_get_enum_context(WsXmlDocH doc)
{
	char           *enumContext = NULL;
	WsXmlNodeH      enumStartNode = ws_xml_get_child(ws_xml_get_soap_body(doc),
			0, NULL, NULL);

	if (enumStartNode) {
		WsXmlNodeH      cntxNode = ws_xml_get_child(enumStartNode, 0,
				XML_NS_ENUMERATION,
				WSENUM_ENUMERATION_CONTEXT);
		enumContext = u_str_clone(ws_xml_get_node_text(cntxNode));
	} else {
		return NULL;
	}
	return enumContext;
}

void
wsenum_free_enum_context(char *enumcontext)
{
	u_free(enumcontext);
}




WsXmlDocH
wsman_build_envelope(WsContextH cntx,
		const char *action,
		const char *reply_to_uri,
		const char *resource_uri,
		const char *to_uri,
		actionOptions *options)
{
	WsXmlNodeH      node;
	char            uuidBuf[100];
	WsXmlNodeH      header;
	WsXmlDocH       doc = ws_xml_create_envelope(ws_context_get_runtime(cntx), NULL);
	if (!doc) 
		return NULL;

	header = ws_xml_get_soap_header(doc);
	generate_uuid(uuidBuf, sizeof(uuidBuf), 0);

	if (reply_to_uri == NULL) {
		reply_to_uri = WSA_TO_ANONYMOUS;
	}
	if (to_uri == NULL) {
		to_uri = WSA_TO_ANONYMOUS;
	}
	if (action != NULL) {
		ws_serialize_str(cntx, header,
			(char *)action, XML_NS_ADDRESSING, WSA_ACTION, 1);
	}

	if (to_uri) {
		ws_serialize_str(cntx, header, (char *)to_uri,
			XML_NS_ADDRESSING, WSA_TO, 1);
	}
	if (resource_uri) {
		ws_serialize_str(cntx, header, (char *)resource_uri,
				XML_NS_WS_MAN, WSM_RESOURCE_URI, 1);
	}
	if (uuidBuf[0] != 0) {
		ws_serialize_str(cntx, header, uuidBuf,
			XML_NS_ADDRESSING, WSA_MESSAGE_ID, 1);
	}
	if (options->timeout) {
		char            buf[20];
		sprintf(buf, "PT%u.%uS", (unsigned int) options->timeout / 1000,
				(unsigned int) options->timeout % 1000);
		ws_serialize_str(cntx, header, buf,
			XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT, 0);
	}
	if (options->max_envelope_size) {
		ws_serialize_uint32(cntx, header, options->max_envelope_size,
				XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE,
				options->flags & FLAG_MUND_MAX_ESIZE);
	}
	if (options->fragment) {
		int mu = 0;
		if ((options->flags & FLAG_MUND_FRAGMENT) ==
				FLAG_MUND_FRAGMENT) 
			mu = 1;
		ws_serialize_str(cntx, header, options->fragment,
				XML_NS_WS_MAN, WSM_FRAGMENT_TRANSFER,
				1);
	}

	node = ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_REPLY_TO, NULL);
	ws_xml_add_child(node, XML_NS_ADDRESSING, WSA_ADDRESS, (char *)reply_to_uri);

  	/* Do not add the selectors to the header for reference instances */
	if (((options->flags & FLAG_CIM_REFERENCES) != FLAG_CIM_REFERENCES) &&
			((options->flags & FLAG_CIM_ASSOCIATORS) != FLAG_CIM_ASSOCIATORS)) {
		wsman_add_selector_from_options(doc, options);

		if (options->cim_ns) {
			wsman_add_selector(header,
					CIM_NAMESPACE_SELECTOR, options->cim_ns);
		}
	}
	return doc;
}





/**
 * Buid Inbound Envelope from Response
 * @param cl Client Handler
 * @return XML document with Envelope
 */

WsXmlDocH
wsman_build_envelope_from_response(WsManClient * cl)
{
	WsXmlDocH       doc = NULL;
	u_buf_t        *buffer = cl->connection->response;

	if (!buffer || !u_buf_ptr(buffer)) {
		error("NULL response");
		return NULL;
	}
	doc = ws_xml_read_memory(ws_context_get_runtime(cl->wscntx),
			u_buf_ptr(buffer), u_buf_len(buffer), NULL, 0);
	if (doc == NULL) {
		error("could not create xmldoc from response");
	}
	return doc;
}


static void
release_connection(WsManConnection * conn)
{
	if (conn == NULL) {
		return;
	}
	if (conn->request) {
		u_buf_free(conn->request);
		conn->request = NULL;
	}
	if (conn->response) {
		u_buf_free(conn->response);
		conn->response = NULL;
	}
	u_free(conn);
}



void
reinit_client_connection(WsManClient * cl) 
{
	u_buf_clear(cl->connection->response);
	u_buf_clear(cl->connection->request);
	cl->response_code = 0;
	cl->last_error = 0;
	if (cl->fault_string) {
		u_free(cl->fault_string);
		cl->fault_string = NULL;
	}
}


static void
init_client_connection(WsManClient * cl)
{
	WsManConnection *conn = (WsManConnection *) u_zalloc(sizeof(WsManConnection));
	u_buf_create(&conn->response);
	u_buf_create(&conn->request);
	cl->response_code = 0;
	cl->connection = conn;
}


WsManClient*
wsman_create_client_from_uri(const char* endpoint) 
{
	u_uri_t *uri = NULL;
	WsManClient* cl;
	if (endpoint != NULL)
		if (u_uri_parse((const char *) endpoint, &uri) != 0 )
			return NULL;
	cl = wsman_create_client( uri->host,
			uri->port,
			uri->path,
			uri->scheme,
			uri->user,
			uri->pwd);

	return cl;
}


int
wsman_client_check_for_fault(WsXmlDocH doc ) 
{
	return wsman_is_fault_envelope(doc);
}


WsManFault *
wsman_client_fault_new(void)
{
	WsManFault     *fault =
		(WsManFault *) u_zalloc(sizeof(WsManFault));

	if (fault)
		return fault;
	else
		return NULL;
}



void
wsman_client_get_fault_data(WsXmlDocH doc, 
		WsManFault *fault)
{
	WsXmlNodeH body;
	WsXmlNodeH fault_node;
	WsXmlNodeH code;
	WsXmlNodeH reason;
	WsXmlNodeH detail;
	if (wsman_client_check_for_fault(doc) == 0 || !fault )
		return;

	body = ws_xml_get_soap_body(doc);	
	fault_node = ws_xml_get_child(body, 0, XML_NS_SOAP_1_2, SOAP_FAULT);
	if (!fault_node) 
		return;

	code = ws_xml_get_child(fault_node, 0, XML_NS_SOAP_1_2, SOAP_CODE);
	if (code) {
		WsXmlNodeH code_v = ws_xml_get_child(code, 0 , XML_NS_SOAP_1_2, SOAP_VALUE);
		WsXmlNodeH subcode = ws_xml_get_child(code, 0 , XML_NS_SOAP_1_2, SOAP_SUBCODE);
		WsXmlNodeH subcode_v = ws_xml_get_child(subcode, 0 , XML_NS_SOAP_1_2, SOAP_VALUE);
		fault->code = ws_xml_get_node_text(code_v);
		fault->subcode = ws_xml_get_node_text(subcode_v);
	}
	reason = ws_xml_get_child(fault_node, 0, XML_NS_SOAP_1_2, SOAP_REASON);
	if (reason) {
		WsXmlNodeH reason_text = ws_xml_get_child(reason, 0 , XML_NS_SOAP_1_2, SOAP_TEXT);
		fault->reason = ws_xml_get_node_text(reason_text);		
	}
	detail = ws_xml_get_child(fault_node, 0, XML_NS_SOAP_1_2, SOAP_DETAIL);
	if (detail) {
		WsXmlNodeH fault_detail = ws_xml_get_child(detail, 0 , XML_NS_WS_MAN, SOAP_FAULT_DETAIL);
		fault->fault_detail = ws_xml_get_node_text(fault_detail);		
	}		
	return;			
}



WsManClient*
wsman_create_client(const char *hostname,
		const int port,
		const char *path,
		const char *scheme,
		const char *username,
		const char *password) 
{
	WsManClient    *wsc = (WsManClient *) calloc(1, sizeof(WsManClient));
	wsc->hdl = &wsc->data;
	if (pthread_mutex_init(&wsc->mutex, NULL)) {
		u_free(wsc);
		return NULL;
	}
	wsc->wscntx = ws_create_runtime(NULL);


	wsc->dumpfile = stdout;
	wsc->data.scheme = strdup(scheme ? scheme : "http");
	wsc->data.hostname = hostname ? strdup(hostname) : strdup("localhost");
	wsc->data.port = port;
	wsc->data.path = strdup(path ? path : "/wsman");
	wsc->data.user = username ? strdup(username) : NULL;
	wsc->data.pwd = password ? strdup(password) : NULL;
	wsc->data.auth_set = 0;
	wsc->initialized = 0;

	wsc->data.endpoint = u_strdup_printf("%s://%s:%d%s",
			scheme, hostname, port, path);
	debug("Endpoint: %s", wsc->data.endpoint);

	init_client_connection(wsc);

	return wsc;
}



void
wsman_release_client(WsManClient * cl)
{

	if (cl->data.scheme) {
		u_free(cl->data.scheme);
		cl->data.scheme = NULL;
	}
	if (cl->data.hostname) {
		u_free(cl->data.hostname);
		cl->data.hostname = NULL;
	}
	if (cl->data.path) {
		u_free(cl->data.path);
		cl->data.path = NULL;
	}
	if (cl->data.user) {
		u_free(cl->data.user);
		cl->data.user = NULL;
	} if (cl->data.pwd) {
		u_free(cl->data.pwd);
		cl->data.pwd = NULL;
	}
	if (cl->data.endpoint) {
		u_free(cl->data.endpoint);
		cl->data.endpoint = NULL;
	}
	if (cl->fault_string) {
		u_free(cl->fault_string);
		cl->fault_string = NULL;
	}
	if (cl->connection) {
		release_connection(cl->connection);
		cl->connection = NULL;
	}
	if (cl->wscntx) {
		SoapH           soap = ws_context_get_runtime(cl->wscntx);
		soap_destroy_fw(soap);
		cl->wscntx = NULL;
	}
	wsman_transport_close_transport(cl);

	u_free(cl);
}

int 
wsman_client_lock(WsManClient * cl)
{
	pthread_mutex_lock(&cl->mutex);
	if (cl->flags & WSMAN_CLIENT_BUSY) {
		pthread_mutex_unlock(&cl->mutex);
		return 1;
	}
	cl->flags |= WSMAN_CLIENT_BUSY;
	pthread_mutex_unlock(&cl->mutex);
	return 0;
}


void 
wsman_client_unlock(WsManClient * cl)
{
	pthread_mutex_lock(&cl->mutex);
	cl->flags &= ~WSMAN_CLIENT_BUSY;
	pthread_mutex_unlock(&cl->mutex);
}


void
wsman_remove_query_string(const char *s, char **result)
{
	char           *r = 0;
	const char     *q;
	char           *buf = 0;

	buf = u_strndup(s, strlen(s));
	if ((q = strchr(buf, '?')) != NULL) {
		r = u_strndup(s, q - buf);
		*result = r;
	} else {
		*result = (char *)s;
	}

	U_FREE(buf);
}
