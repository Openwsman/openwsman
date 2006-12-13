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
#include "wsman-xml-serializer.h"
#include "wsman-client-transport.h"
#include "wsman-faults.h"
#include "wsman-client.h"


WsContextH
wsman_create_runtime (void)
{
    return ws_create_runtime(NULL);
}


WsXmlDocH
wsman_create_doc(WsContextH cntx, char *rootname)
{
    return ws_xml_create_doc(cntx->soap, NULL, rootname);
}

//Obsoleted
long
wsman_get_client_response_code(WsManClient * cl)
{
    return wsman_client_get_response_code(cl);
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

WsXmlDocH
wsman_client_read_file(WsManClient * cl, char *filename,
               char *encoding, unsigned long options)
{
    return ws_xml_read_file(ws_context_get_runtime(cl->wscntx),
                filename, encoding, options);
}

WsXmlDocH
wsman_client_read_memory(WsManClient * cl, char *buf,
             int size, char *encoding, unsigned long options)
{
    return ws_xml_read_memory(ws_context_get_runtime(cl->wscntx),
                  buf, size, encoding, options);
}

void
initialize_action_options(actionOptions * op)
{
    bzero(op, sizeof(actionOptions));
    op->selectors = NULL;
    op->max_elements = 0;
    return;
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
    bzero(op, sizeof(actionOptions));
    return;
}

void
wsman_set_action_option(actionOptions * options, unsigned int flag)
{
    options->flags |= flag;
    return;
}


void
wsman_remove_query_string(char *s, char **result)
{
    char           *r = 0;
    const char     *q;
    char           *buf = 0;

    buf = u_strndup(s, strlen(s));
    if ((q = strchr(buf, '?')) != NULL) {
        r = u_strndup(s, q - buf);
        *result = r;
    } else {
        *result = s;
    }

    U_FREE(buf);
}

hash_t*
wsman_create_hash_from_query_string(const char *query_string)
{
    if (query_string) {
        hash_t         *query = parse_query(query_string);
        if (query) {
            return (query);
        }
    }
    return NULL;
}

void
wsman_client_add_property(actionOptions * options,
              char *key,
              char *value)
{
    if (options->properties == NULL)
        options->properties = hash_create(HASHCOUNT_T_MAX, 0, 0);
    if (!hash_lookup(options->properties, key)) {
        if (!hash_alloc_insert(options->properties, key, value)) {
            fprintf(stderr, "hash_alloc_insert failed");
        }
    } else {
        fprintf(stderr, "duplicate not added to hash");
    }
}

void
wsman_client_add_selector(actionOptions * options,
              char *key,
              char *value)
{
    if (options->selectors == NULL)
        options->selectors = hash_create(HASHCOUNT_T_MAX, 0, 0);
    if (!hash_lookup(options->selectors, key)) {
        if (!hash_alloc_insert(options->selectors, key, value)) {
            fprintf(stderr, "hash_alloc_insert failed");
        }
    } else {
        fprintf(stderr, "duplicate not added to hash");
    }
}

void
wsman_add_selectors_from_query_string(actionOptions * options,
                      const char *query_string)
{
    if (query_string) {
        hash_t         *query = parse_query(query_string);
        if (query) {
            options->selectors = query;
        }
    }
}

void
wsman_add_properties_from_query_string(actionOptions * options,
                       const char *query_string)
{
    if (query_string) {
        hash_t         *query = parse_query(query_string);
        if (query) {
            options->properties = query;
        }
    }
}

void
wsman_add_selector_from_options(WsXmlDocH doc, actionOptions options)
{
    if (options.selectors != NULL && hash_count(options.selectors) > 0) {
        WsXmlNodeH      header = ws_xml_get_soap_header(doc);
        hnode_t        *hn;
        hscan_t         hs;
        hash_scan_begin(&hs, options.selectors);
        while ((hn = hash_scan_next(&hs))) {
            wsman_add_selector(header,
             (char *) hnode_getkey(hn), (char *) hnode_get(hn));
            debug("key = %s value=%s",
             (char *) hnode_getkey(hn), (char *) hnode_get(hn));
        }
    }
}


void
wsman_add_namespace_as_selector(WsXmlDocH doc,
                char *_namespace)
{
    WsXmlNodeH      header = ws_xml_get_soap_header(doc);
    wsman_add_selector(header,
               CIM_NAMESPACE_SELECTOR, _namespace);

    return;
}



void
wsman_set_options_from_uri(char *resource_uri, actionOptions * options)
{
    u_uri_t        *uri;
    if (resource_uri != NULL) {
        if (u_uri_parse((const char *) resource_uri, &uri) != 0)
            return;
    } else {
        return;
    }
    if (uri->query != NULL) {
        wsman_add_selectors_from_query_string(options, uri->query);
    }
    if (uri) {
        u_uri_free(uri);
    }
}

void
wsman_add_selector_from_uri(WsXmlDocH doc,
                char *resource_uri)
{
    u_uri_t        *uri;
    WsXmlNodeH      header = ws_xml_get_soap_header(doc);

    if (resource_uri != NULL)
        if (u_uri_parse((const char *) resource_uri, &uri) != 0)
            return;

    if (uri->query != NULL) {
        hash_t         *query = parse_query(uri->query);
        hnode_t        *hn;
        hscan_t         hs;
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
    }
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

char*
wsman_make_action(char *uri, char *op_name)
{
    int             len = strlen(uri) + strlen(op_name) + 2;
    char           *ptr = (char *) malloc(len);
    if (ptr) {
        sprintf(ptr, "%s/%s", uri, op_name);
    }
    return ptr;
}
/*
static void
wsman_set_transfer_create_properties(WsXmlDocH request,
                     actionOptions options)
{
    hscan_t         hs;
    hnode_t        *hn;
    char           *resource_uri = NULL, *class = NULL;
    WsXmlNodeH      body = ws_xml_get_soap_body(request);

    WsXmlNodeH      resource = ws_xml_add_child(body, resource_uri, class, NULL);

    if (!options.properties) {
        return;
    }
    hash_scan_begin(&hs, options.properties);
    while ((hn = hash_scan_next(&hs))) {
        ws_xml_add_child(resource,
                 resource_uri, (char *) hnode_getkey(hn), (char *) hnode_get(hn));
    }
}
*/




static void
wsman_set_enumeration_options(WsXmlNodeH body, actionOptions options)
{
    WsXmlNodeH      node = ws_xml_get_child(body, 0, NULL, NULL);
    if ((options.flags & FLAG_ENUMERATION_OPTIMIZATION) ==
        FLAG_ENUMERATION_OPTIMIZATION) {
        ws_xml_add_child(node, XML_NS_WS_MAN, WSM_OPTIMIZE_ENUM, NULL);
        if (options.max_elements > 0) {
            ws_xml_add_child_format(node, XML_NS_WS_MAN,
               WSENUM_MAX_ELEMENTS, "%d", options.max_elements);
        }
    }
    if ((options.flags & FLAG_ENUMERATION_ENUM_EPR) ==
        FLAG_ENUMERATION_ENUM_EPR) {
        ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE, WSM_ENUM_EPR);
    } else if ((options.flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) ==
           FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) {
        ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE,
                 WSM_ENUM_OBJ_AND_EPR);
    }
    if ((options.flags &
    FLAG_IncludeSubClassProperties) == FLAG_IncludeSubClassProperties) {
        ws_xml_add_child(node, XML_NS_CIM_BINDING,
            WSMB_POLYMORPHISM_MODE, WSMB_INCLUDE_SUBCLASS_PROP);
    } else if ((options.flags &
    FLAG_ExcludeSubClassProperties) == FLAG_ExcludeSubClassProperties) {
        ws_xml_add_child(node, XML_NS_CIM_BINDING,
            WSMB_POLYMORPHISM_MODE, WSMB_EXCLUDE_SUBCLASS_PROP);
    } else if ((options.flags &
            FLAG_POLYMORPHISM_NONE) == FLAG_POLYMORPHISM_NONE) {
        ws_xml_add_child(node, XML_NS_CIM_BINDING,
                 WSMB_POLYMORPHISM_MODE, "None");
    }
}

static void
wsman_set_transfer_put_properties(WsXmlDocH get_response,
                  WsXmlDocH put_request,
                  actionOptions options)
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

    if (!options.properties) {
        return;
    }
    hash_scan_begin(&hs, options.properties);
    while ((hn = hash_scan_next(&hs))) {
        WsXmlNodeH      n = ws_xml_get_child(resource_node, 0,
                     ns_uri, (char *) hnode_getkey(hn));
        ws_xml_set_node_text(n, (char *) hnode_get(hn));
    }
}





WsXmlDocH
wsman_client_create_request(WsManClient * cl,
                WsmanAction action,
                char *method,
                char *resource_uri,
                actionOptions options,
                void *data)
{
    WsXmlDocH       request;
    WsXmlNodeH      body;
    WsXmlNodeH      header;
    WsXmlNodeH      node;
    hscan_t         hs;
    hnode_t        *hn;
    char           *_action = NULL;
    WsXmlNodeH      filter;

    if (action == WSMAN_ACTION_IDENTIFY) {
        request = ws_xml_create_envelope(
                  ws_context_get_runtime(cl->wscntx), NULL);
    } else {
        if (method) {
            if (strchr(method, '/')) {
                _action = u_strdup(method);
            } else {
                _action = wsman_make_action(resource_uri, method);
            }
        } else {
            _action = wsman_create_action_str(action);
        }

        request = wsman_build_envelope(cl->wscntx, _action,
                         WSA_TO_ANONYMOUS, resource_uri,
                           cl->data.endpoint, options);
    }

    body = ws_xml_get_soap_body(request);
    header = ws_xml_get_soap_header(request);


    switch (action) {
    case WSMAN_ACTION_IDENTIFY:
        ws_xml_add_child(ws_xml_get_soap_body(request),
                 XML_NS_WSMAN_ID, WSMID_IDENTIFY, NULL);
        break;
    case WSMAN_ACTION_CUSTOM:
        if (options.properties) {
            if (method) {
                node = ws_xml_add_empty_child_format(body, resource_uri, "%s_INPUT", method);
                hash_scan_begin(&hs, options.properties);
                while ((hn = hash_scan_next(&hs))) {
                    ws_xml_add_child(node, NULL, (char *) hnode_getkey(hn),
                            (char *) hnode_get(hn));
                }
            }
        } else {
            ws_xml_add_empty_child_format(body, resource_uri, "%s_INPUT", method);
        }
        break;
    case WSMAN_ACTION_TRANSFER_PUT:
        wsman_set_transfer_put_properties((WsXmlDocH) data, request, options);
        break;  
    case WSMAN_ACTION_ENUMERATION:
        node = ws_xml_add_child(ws_xml_get_soap_body(request),
                XML_NS_ENUMERATION, WSENUM_ENUMERATE, NULL);
        wsman_set_enumeration_options(body, options);
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
    case WSMAN_ACTION_TRANSFER_DELETE:
        break;
    }

    if (action == WSMAN_ACTION_PULL || action == WSMAN_ACTION_ENUMERATION) {
        if (options.max_elements > 0) {
            node = ws_xml_get_child(body, 0, NULL, NULL);
            ws_xml_add_child_format(node, XML_NS_ENUMERATION,
               WSENUM_MAX_ELEMENTS, "%d", options.max_elements);
        }
        if ((options.flags & FLAG_ENUMERATION_COUNT_ESTIMATION) ==
            FLAG_ENUMERATION_COUNT_ESTIMATION) {
            ws_xml_add_child(header, XML_NS_WS_MAN, WSM_REQUEST_TOTAL, NULL);
        }
        if (options.filter && options.dialect) {
            node = ws_xml_get_child(body, 0, NULL, NULL);
            filter = ws_xml_add_child(node,
                  XML_NS_WS_MAN, WSENUM_FILTER, options.filter);
            ws_xml_add_node_attr(filter, NULL, WSENUM_DIALECT, options.dialect);
        }
    }
    if (action != WSMAN_ACTION_TRANSFER_CREATE) {
        if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(request));
        }
    }
    return request;
}

WsXmlDocH
ws_transfer_create(WsManClient * cl,
           char *resource_uri,
           void *data,
           void* typeInfo,         
           actionOptions options)
{
    WsXmlDocH       response;
    WsXmlDocH       request = wsman_client_create_request(cl, WSMAN_ACTION_TRANSFER_CREATE,
                     NULL, resource_uri, options, NULL);
                
    if (data && typeInfo) {     
        char *class = u_strdup(strrchr(resource_uri, '/') + 1);
        
        ws_serialize(cl->wscntx, ws_xml_get_soap_body(request), data, (XmlSerializerInfo *)typeInfo,
             class , resource_uri, resource_uri, 1);
        ws_serializer_free_mem(cl->wscntx, data, (XmlSerializerInfo *)typeInfo);
        if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(request));
        }
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
ws_transfer_delete(WsManClient * cl,
           char *resource_uri,
           actionOptions options)
{

    WsXmlDocH       response;
    WsXmlDocH       request = wsman_client_create_request(cl, WSMAN_ACTION_TRANSFER_DELETE,
                     NULL, resource_uri, options, NULL);
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
        char *resource_uri,
        actionOptions options)
{
    WsXmlDocH       response;
    WsXmlDocH       request = wsman_client_create_request(cl, WSMAN_ACTION_TRANSFER_GET,
                     NULL, resource_uri, options, NULL);
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
        char *resource_uri,
        actionOptions options)
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
    put_request = wsman_client_create_request(cl, WSMAN_ACTION_TRANSFER_PUT,
            NULL, resource_uri, options, (void *) get_response);
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
         char *resource_uri,
         char *method,
         actionOptions options)
{
    WsXmlDocH       response;
    WsXmlDocH       request = wsman_client_create_request(cl, WSMAN_ACTION_CUSTOM, method,
                           resource_uri, options, NULL);
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
           actionOptions options)
{
    WsXmlDocH       response;
    WsXmlDocH       request = wsman_client_create_request(cl, WSMAN_ACTION_IDENTIFY,
                         NULL, NULL, options, NULL);
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
              char *resource_uri,
              actionOptions options,
              SoapResponseCallback callback,
              void *callback_data)
{
    WsXmlDocH       doc;
    char           *enumContext;
    WsXmlDocH       enum_response = wsenum_enumerate(cl,
                             resource_uri, options);


    if (enum_response) {
        if (wsman_get_client_response_code(cl) == 200 ||
            wsman_get_client_response_code(cl) == 400 ||
            wsman_get_client_response_code(cl) == 500) {
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
        doc = wsenum_pull(cl, resource_uri, enumContext, options);

        if (wsman_get_client_response_code(cl) != 200 &&
            wsman_get_client_response_code(cl) != 400 &&
            wsman_get_client_response_code(cl) != 500) {
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
         char *resource_uri,
         actionOptions options)
{
    WsXmlDocH       response;
    WsXmlDocH       request = wsman_client_create_request(cl, WSMAN_ACTION_ENUMERATION,
                     NULL, resource_uri, options, NULL);
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
        char *resource_uri,
        char *enumContext,
        actionOptions options)
{
    WsXmlDocH       response;
    WsXmlNodeH      node;

    if (enumContext || (enumContext && enumContext[0] == 0)) {
        WsXmlDocH       request = wsman_client_create_request(cl, WSMAN_ACTION_PULL, NULL,
                    resource_uri, options, enumContext);
        if (wsman_send_request(cl, request)) {
            ws_xml_destroy_doc(request);
            u_free(enumContext);
            return NULL;
        }
        response = wsman_build_envelope_from_response(cl);
        u_free(enumContext);
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
           char *resource_uri,
           char *enumContext,
           actionOptions options)
{
    WsXmlDocH       response;

    if (enumContext || (enumContext && enumContext[0] == 0)) {
        WsXmlDocH       request = wsman_client_create_request(cl, WSMAN_ACTION_RELEASE,
                  NULL, resource_uri, options, enumContext);
        if (wsman_send_request(cl, request)) {
            ws_xml_destroy_doc(request);
            u_free(enumContext);
            return NULL;
        }
        response = wsman_build_envelope_from_response(cl);
        u_free(enumContext);
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



WsXmlDocH
wsman_build_envelope(WsContextH cntx,
             char *action,
             char *reply_to_uri,
             char *resource_uri,
             char *to_uri,
             actionOptions options)
{
    WsXmlNodeH      node;
    char            uuidBuf[100];
    WsXmlNodeH      header;
    unsigned long   savedMustUnderstand;
    WsXmlDocH       doc = ws_xml_create_envelope(ws_context_get_runtime(cntx), NULL);
    if (!doc) {
        return NULL;
    }
    savedMustUnderstand = ws_get_context_ulong_val(cntx,
                           ENFORCE_MUST_UNDERSTAND);
    header = ws_xml_get_soap_header(doc);

    generate_uuid(uuidBuf, sizeof(uuidBuf), 0);
    ws_set_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND, 1);

    if (reply_to_uri == NULL) {
        reply_to_uri = WSA_TO_ANONYMOUS;
    }
    if (to_uri == NULL) {
        to_uri = WSA_TO_ANONYMOUS;
    }
    if (action != NULL) {
        ws_serialize_str(cntx, header, action, XML_NS_ADDRESSING, WSA_ACTION);
    }
    u_free(action);

    if (to_uri) {
        ws_serialize_str(cntx, header, to_uri, XML_NS_ADDRESSING, WSA_TO);
    }
    if (resource_uri) {
        ws_serialize_str(cntx, header, resource_uri,
                 XML_NS_WS_MAN, WSM_RESOURCE_URI);
    }
    if (uuidBuf[0] != 0) {
        ws_serialize_str(cntx, header, uuidBuf, XML_NS_ADDRESSING, WSA_MESSAGE_ID);
    }
    if (options.timeout) {
        char            buf[20];
        sprintf(buf, "PT%u.%uS", (unsigned int) options.timeout / 1000,
            (unsigned int) options.timeout % 1000);
        ws_serialize_str(cntx, header, buf, XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT);
    }
    if (options.max_envelope_size) {
        ws_serialize_uint32(cntx, header, options.max_envelope_size,
                    XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE);
    }
    if (options.fragment) {
        ws_serialize_str(cntx, header, options.fragment,
                 XML_NS_WS_MAN, WSM_FRAGMENT_TRANSFER);
    }
    ws_set_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND, savedMustUnderstand);

    node = ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_REPLY_TO, NULL);
    ws_xml_add_child(node, XML_NS_ADDRESSING, WSA_ADDRESS, reply_to_uri);

    wsman_add_selector_from_options(doc, options);

    if (options.cim_ns) {
        wsman_add_selector(header,
                   CIM_NAMESPACE_SELECTOR, options.cim_ns);
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
    u_buf_t        *buffer;

    SoapH           soap = ws_context_get_runtime(cl->wscntx);
    char           *response = (char *) u_buf_ptr(cl->connection->response);
    if (response) {
        if (u_buf_create(&buffer) != 0) {
            error("Error while creating buffer");
        } else {
            u_buf_set(buffer, response, strlen(response));
        }

        doc = ws_xml_read_memory(soap, u_buf_ptr(buffer),
                     u_buf_size(buffer), NULL, 0);
        if (doc != NULL) {
            debug("xml doc received...");
        }
        u_buf_free(buffer);
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

    wsc->data.hostName = hostname ? strdup(hostname) : strdup("localhost");
    wsc->data.port = port;
    wsc->data.path = strdup(path ? path : "/wsman");
    wsc->data.user = username ? strdup(username) : NULL;
    wsc->data.pwd = password ? strdup(password) : NULL;


    wsc->data.endpoint = u_strdup_printf("%s://%s:%d%s",
                         scheme, hostname, port, path);
    debug("Endpoint: %s", wsc->data.endpoint);

    init_client_connection(wsc);

    return wsc;
}



void
wsman_release_client(WsManClient * cl)
{

    if (cl->data.hostName) {
        u_free(cl->data.hostName);
        cl->data.hostName = NULL;
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
