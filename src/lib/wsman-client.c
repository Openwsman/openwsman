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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-dispatcher.h"
#include "wsman-soap-envelope.h"

#include "wsman-xml.h"
#include "wsman-xml-serializer.h" 
#include <libxml/uri.h>
#include "wsman-faults.h"
#include "wsman-client.h"



void initialize_action_options(actionOptions *op) 
{
   	bzero(op, sizeof(actionOptions));
	op->selectors = NULL;
	return;
}

void destroy_action_options(actionOptions *op) 
{
	if (op->selectors) {		
		hash_free(op->selectors);		
	}
   	bzero(op, sizeof(actionOptions));
	return;
}

void 
wsman_remove_query_string(char *s, char **result)
{
    char *r = 0;
	const char *q;
	char *buf = 0;
	
	buf = u_strndup(s, strlen(s));
	if ( (q = strchr (buf, '?')) != NULL) {
		r = u_strndup(s, q - buf);		
		*result = r;
	} else {
		*result = s;
	}
	
	U_FREE(buf);	   
}



void
wsman_add_selectors_from_query_string( actionOptions *options, 
									   const char *query_string)
{
	if (query_string) {
		hash_t * query = parse_query(query_string);	
		if (query) {
			options->selectors = query;
		}
	}
}

void 
wsman_add_selector_from_options( WsXmlDocH doc, actionOptions options)
{
    if (options.selectors != NULL && hash_count(options.selectors) > 0 ) 
    {
        WsXmlNodeH header = ws_xml_get_soap_header(doc);
        hnode_t *hn;
        hscan_t hs;
        hash_scan_begin(&hs, options.selectors);
        while ((hn = hash_scan_next(&hs))) {
            wsman_add_selector( header,
                    (char*) hnode_getkey(hn), (char*) hnode_get(hn));
            debug("key = %s value=%s",
                    (char*)hnode_getkey(hn), (char*)hnode_get(hn));
        }
    }
    //hash_free(query);
    //hash_destroy(query);
}


// FIXME
void
wsman_add_namespace_as_selector( WsXmlDocH doc, 
                                 char *_namespace)
{
    WsXmlNodeH header = ws_xml_get_soap_header(doc);
    WsXmlNodeH set = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);
    debug("%p",  set);
    if (set != NULL) {
       //set = ws_xml_add_child(header, XML_NS_WS_MAN, WSM_SELECTOR_SET, NULL);
    }
    return;
}



void 
wsman_set_options_from_uri( char *resourceUri, actionOptions *options)
{
    u_uri_t *uri;
    if (resourceUri != NULL ) {
        u_uri_parse((const char *)resourceUri, &uri);
    } else {
        return;
    }
    if (uri->query != NULL  ) 
    {
        wsman_add_selectors_from_query_string(options, uri->query);
    }
    if (uri) {
        u_uri_free(uri);
    }
}


void 
wsman_add_selector_from_uri( WsXmlDocH doc, 
                             char *resourceUri)
{
    u_uri_t *uri;
    WsXmlNodeH header = ws_xml_get_soap_header(doc);

    if (resourceUri != NULL )	
        u_uri_parse((const char *)resourceUri, &uri);

    if (uri->query != NULL  ) {
        hash_t * query = parse_query(uri->query);
        hnode_t *hn;
        hscan_t hs;
        hash_scan_begin(&hs, query);
        while ((hn = hash_scan_next(&hs))) {
            wsman_add_selector( header, 
                    (char*) hnode_getkey(hn), 
                    (char*) hnode_get(hn));
            debug("key=%s value=%s", (char*)hnode_getkey(hn),
                                            (char*)hnode_get(hn));
        }
        hash_free_nodes(query);
        hash_destroy(query);
    }
    if (uri)
        u_uri_free(uri);
}


char*
wsman_make_action( char* uri, 
                   char* opName)
{
    int len = strlen(uri) + strlen(opName) + 2;
    char* ptr = (char*)malloc(len);
    if ( ptr )
        sprintf(ptr, "%s/%s", uri, opName);
    return ptr;
}


WsXmlDocH ws_transfer_create( WsManClient *cl,
                           char *resourceUri,
                           hash_t *prop,
                           actionOptions options)
{
    return NULL;
}

WsXmlDocH 
ws_transfer_get( WsManClient *cl,
              char *resource_uri,
              actionOptions options) 
{
    //char *clean_uri = NULL;
    WsXmlDocH respDoc = NULL;    
    char *action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);

    WsXmlDocH rqstDoc = wsman_build_envelope(cl->wscntx,
                            action, WSA_TO_ANONYMOUS, 
                            resource_uri , cl->data.endpoint, options );

    wsman_add_selector_from_options(rqstDoc, options);

    if (options.cim_ns) {
        wsman_add_selector(ws_xml_get_soap_header(rqstDoc),
                            CIM_NAMESPACE_SELECTOR, options.cim_ns);
    }
    if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc));
    }
    respDoc = ws_send_get_response(cl, rqstDoc, options.timeout);

    ws_xml_destroy_doc(rqstDoc);
    u_free(action);

    release_connection(cl->connection);
    return respDoc;

}




WsXmlDocH
ws_transfer_put( WsManClient *cl,
              char *resourceUri,
              hash_t *prop,
              actionOptions options) 
{
    char *r = NULL;

    wsman_remove_query_string(resourceUri, &r);
    char *action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);
    WsXmlDocH get_respDoc = NULL;
    WsXmlDocH respDoc = NULL;

   
    WsXmlDocH get_rqstDoc = wsman_build_envelope(cl->wscntx, action,
                                WSA_TO_ANONYMOUS,  r,
                                cl->data.endpoint, options);
    u_free(action);


    wsman_add_selector_from_options(get_rqstDoc, options);
    get_respDoc = ws_send_get_response(cl, get_rqstDoc, options.timeout);

    WsXmlNodeH get_body = ws_xml_get_soap_body(get_respDoc);

    action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_PUT);
    WsXmlDocH put_rqstDoc = wsman_build_envelope(cl->wscntx,
                                    action, WSA_TO_ANONYMOUS, r,
                                    cl->data.endpoint, options);

    wsman_add_selector_from_options(put_rqstDoc, options);

    if (options.cim_ns)
        wsman_add_selector(ws_xml_get_soap_header(put_rqstDoc),
                                CIM_NAMESPACE_SELECTOR, options.cim_ns);

    WsXmlNodeH put_body = ws_xml_get_soap_body(put_rqstDoc);
    ws_xml_duplicate_tree(put_body, ws_xml_get_child(get_body, 0, NULL, NULL));

    WsXmlNodeH resource_node = ws_xml_get_child(put_body, 0 , NULL, NULL);
    char *nsUri = ws_xml_get_node_name_ns_uri(resource_node);
    hscan_t hs;
    hnode_t *hn;
    hash_scan_begin(&hs, prop);
    while ((hn = hash_scan_next(&hs))) {    	
        WsXmlNodeH n = ws_xml_get_child(resource_node, 0,
                                        nsUri ,(char*) hnode_getkey(hn) );
        ws_xml_set_node_text(n, (char*) hnode_get(hn));
    }

    if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(put_rqstDoc));
    }
    respDoc = ws_send_get_response(cl, put_rqstDoc, options.timeout);

    ws_xml_destroy_doc(get_rqstDoc);
    ws_xml_destroy_doc(put_rqstDoc);
    u_free(action);
	u_free(r);
	
	release_connection(cl->connection);
    return respDoc;

}


WsXmlDocH
wsman_invoke( WsManClient *cl,
        char *resourceUri,
        char *method,
        hash_t *prop,
        actionOptions options)
{
    WsXmlDocH respDoc = NULL;
	char* uri = NULL;
    hscan_t hs;
    hnode_t *hn;	
    
 	WsXmlNodeH argsin = NULL;

    char *action = NULL;
    wsman_remove_query_string(resourceUri, &uri);
    if (strchr(method, '/')) {
        action = method;
    } else {
        action = wsman_make_action(uri , method );
    }
    WsXmlDocH rqstDoc = wsman_build_envelope(cl->wscntx,
                                        action, WSA_TO_ANONYMOUS, 
                                        uri , cl->data.endpoint, options );

    //wsman_add_selector_from_uri( rqstDoc, resourceUri);
    wsman_add_selector_from_options(rqstDoc, options);
    if (options.cim_ns)
        wsman_add_selector(ws_xml_get_soap_header(rqstDoc),
                                CIM_NAMESPACE_SELECTOR, options.cim_ns);


    if (prop)
        argsin = ws_xml_add_empty_child_format(
                ws_xml_get_soap_body(rqstDoc), uri , "%s_INPUT", method);

    hash_scan_begin(&hs, prop);
    while ((hn = hash_scan_next(&hs))) {
        ws_xml_add_child(argsin, NULL, (char*) hnode_getkey(hn),
                                                    (char*) hnode_get(hn));
    }

    if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
        if (rqstDoc)
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc));
    }

    respDoc = ws_send_get_response(cl, rqstDoc, options.timeout);
    ws_xml_destroy_doc(rqstDoc);
    if (action) 
        u_free(action);
	if (uri)
    	u_free(uri);

	release_connection(cl->connection);
    return respDoc;

}

WsXmlDocH
wsman_identify( WsManClient *cl,
                actionOptions options)
{	
    WsXmlDocH respDoc = NULL;
   
    WsXmlDocH doc = ws_xml_create_envelope(
                            ws_context_get_runtime(cl->wscntx), NULL);

    ws_xml_add_child(ws_xml_get_soap_body(doc),
                                XML_NS_WSMAN_ID, WSMID_IDENTIFY , NULL);
    
        
    respDoc = ws_send_get_response(cl, doc, options.timeout);
    ws_xml_destroy_doc(doc);
	release_connection(cl->connection);
	
    return respDoc;
}



WsXmlDocH
wsenum_enumerate( WsManClient* cl,
                  char *resourceUri,
                  int max_elements , 
                  actionOptions options) 
{	
    message( "Enumerate request...");
    WsXmlDocH respDoc = wsman_enum_send_get_response(cl, WSENUM_ENUMERATE,
								NULL, resourceUri, max_elements, options);	
    return respDoc;
}



WsXmlDocH
wsenum_pull( WsManClient* cl,
             char *resourceUri,
             char *enumContext,
             int max_elements,
             actionOptions options)
{
    WsXmlDocH respDoc;

    message( "Pull request...");   	
    if ( enumContext || (enumContext && enumContext[0] == 0) )
	{
        respDoc = wsman_enum_send_get_response(cl, WSENUM_PULL, 
        enumContext, resourceUri, max_elements, options);		
        u_free(enumContext);
    } else {
         error( "No enumeration context ???");
        return NULL;
    }

    WsXmlNodeH node = ws_xml_get_child(ws_xml_get_soap_body(respDoc),
                    0, NULL, NULL);

    if (node == NULL) {
        error( "no Pull response" );
        return NULL;
    }
    if ( strcmp(ws_xml_get_node_local_name(node),
                                     WSENUM_PULL_RESP) != 0 ) {
        error( "no Pull response" );
        return NULL;
    }
    if ( ws_xml_get_child(node, 0, XML_NS_ENUMERATION,
                                     WSENUM_END_OF_SEQUENCE) ) {
        debug( "End of sequence");
    }

    return respDoc;
}

WsXmlDocH
wsenum_release( WsManClient* cl, 
                char *resourceUri,
                char *enumContext, 
                actionOptions options) 
{
	
	
    WsXmlDocH respDoc = wsman_enum_send_get_response(cl, 
				WSENUM_RELEASE, enumContext, resourceUri, 0 , options);
	release_connection(cl->connection);
    return respDoc;
}


char*
wsenum_get_enum_context(WsXmlDocH doc)
{
    char* enumContext = NULL;
    WsXmlNodeH enumStartNode = ws_xml_get_child(ws_xml_get_soap_body(doc),
                                                             0, NULL, NULL);

    if ( enumStartNode ) {
        WsXmlNodeH cntxNode = ws_xml_get_child(enumStartNode,
                        0, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
        enumContext = u_str_clone(ws_xml_get_node_text(cntxNode));
    } else {
        return NULL;
    }
    return enumContext;
}



WsXmlDocH 
wsman_build_envelope( WsContextH cntx,
                      char* action, 
                      char* replyToUri,                      
                      char* resourceUri, 
                      char* toUri, 
                      actionOptions options)
{
    WsXmlNodeH node;
    char uuidBuf[100];
    
    WsXmlDocH doc = ws_xml_create_envelope(ws_context_get_runtime(cntx), NULL);
    if ( !doc ) {
		return NULL;
	}
	
	
	unsigned long savedMustUnderstand = ws_get_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND);
	WsXmlNodeH header = ws_xml_get_soap_header(doc);
	
	generate_uuid(uuidBuf, sizeof(uuidBuf), 0);
	ws_set_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND, 1);

	if ( replyToUri == NULL )
		replyToUri = WSA_TO_ANONYMOUS;

	if ( toUri == NULL )
		toUri = WSA_TO_ANONYMOUS;

	if ( action )
		ws_serialize_str(cntx, header, action, XML_NS_ADDRESSING, WSA_ACTION); 

	if ( toUri )
		ws_serialize_str(cntx, header, toUri, XML_NS_ADDRESSING, WSA_TO); 

	if ( resourceUri )
		ws_serialize_str(cntx, header, resourceUri, XML_NS_WS_MAN, WSM_RESOURCE_URI); 

    if (uuidBuf[0] != 0)
	    ws_serialize_str(cntx, header, uuidBuf, XML_NS_ADDRESSING, WSA_MESSAGE_ID);

	if ( options.timeout )
	{
		char buf[20];
		sprintf(buf, "PT%u.%uS", (unsigned int)options.timeout/1000, (unsigned int)options.timeout % 1000);
		ws_serialize_str(cntx, header, buf, XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT);
	}

	if ( options.max_envelope_size)
	{
		ws_serialize_uint32(cntx, header, options.max_envelope_size, XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE); 
	}
	
	if ( options.fragment)
	{
		ws_serialize_str(cntx, header, options.fragment, XML_NS_WS_MAN, WSM_FRAGMENT_TRANSFER); 
	}

	ws_set_context_ulong_val(cntx, ENFORCE_MUST_UNDERSTAND, savedMustUnderstand);

	node = ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_REPLY_TO, NULL);
	ws_xml_add_child(node, XML_NS_ADDRESSING, WSA_ADDRESS, replyToUri);

    return doc;
}


WsXmlDocH
wsman_make_enum_message(WsContextH soap,
                        char* op, 
                        char* enumContext, 
                        char* resourceUri, 
                        char* url, 
                        actionOptions options)
{
    WsXmlDocH doc;
    char* action = wsman_make_action(XML_NS_ENUMERATION, op);
    
    doc = wsman_build_envelope(soap, action,
                        WSA_TO_ANONYMOUS, resourceUri, url, options );

    if ( (doc = wsman_build_envelope(soap, action, 
                WSA_TO_ANONYMOUS, resourceUri, url, options )) != NULL )
    {
        WsXmlNodeH node = ws_xml_add_child(ws_xml_get_soap_body(doc),
                                            XML_NS_ENUMERATION, op, NULL);
        if ( enumContext ) {
            ws_xml_add_child(node, XML_NS_ENUMERATION,
                                 WSENUM_ENUMERATION_CONTEXT, enumContext);
        }
    }
    u_free(action);
    return doc;
}



WsXmlDocH
wsman_enum_send_get_response(WsManClient *cl,
                             char* op,
                             char* enumContext,
                             char* resourceUri,
                             int max_elements,
                             actionOptions options)
{
    WsXmlDocH respDoc  = NULL;
    WsXmlNodeH header, body;
    
    WsXmlDocH rqstDoc = wsman_make_enum_message( cl->wscntx, op, enumContext,
                                     resourceUri, cl->data.endpoint, options);

    header = ws_xml_get_soap_header(rqstDoc);
    body = ws_xml_get_soap_body(rqstDoc);
    
    if (options.cim_ns) {
        wsman_add_selector(header, CIM_NAMESPACE_SELECTOR, options.cim_ns);
    }
    if ((options.flags & FLAG_ENUMERATION_COUNT_ESTIMATION) ==
                                        FLAG_ENUMERATION_COUNT_ESTIMATION) 
    {       
        ws_xml_add_child(header, XML_NS_WS_MAN, WSM_REQUEST_TOTAL, NULL);
    }

    if (strcmp(op, WSENUM_ENUMERATE) == 0 ) 
    {
        WsXmlNodeH node = ws_xml_get_child(body,0 , NULL, NULL);
        if ((options.flags & FLAG_ENUMERATION_OPTIMIZATION) ==
                                    FLAG_ENUMERATION_OPTIMIZATION) {
            ws_xml_add_child(node, XML_NS_WS_MAN, WSM_OPTIMIZE_ENUM, NULL);
            if (max_elements) {
                ws_xml_add_child_format(node , XML_NS_WS_MAN,
                            WSENUM_MAX_ELEMENTS, "%d", max_elements);
            }
        }
        if ((options.flags & FLAG_ENUMERATION_ENUM_EPR) ==
                                        FLAG_ENUMERATION_ENUM_EPR)
        {
            ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE, WSM_ENUM_EPR);
        }
        else if ((options.flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) ==
                                        FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) 
        {
            ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE,
                                                WSM_ENUM_OBJ_AND_EPR);
        }

        if ((options.flags & FLAG_IncludeSubClassProperties) ==
                                        FLAG_IncludeSubClassProperties) 
        {
            ws_xml_add_child(node, XML_NS_CIM_BINDING,
                        WSMB_POLYMORPHISM_MODE, WSMB_INCLUDE_SUBCLASS_PROP);
        } else if ((options.flags & FLAG_ExcludeSubClassProperties) ==
                                              FLAG_ExcludeSubClassProperties) 
        {
            ws_xml_add_child(node, XML_NS_CIM_BINDING,
                    WSMB_POLYMORPHISM_MODE, WSMB_EXCLUDE_SUBCLASS_PROP);
        } else if ((options.flags & FLAG_POLYMORPHISM_NONE) ==
                                                    FLAG_POLYMORPHISM_NONE)
        {
            ws_xml_add_child(node, XML_NS_CIM_BINDING,
                                            WSMB_POLYMORPHISM_MODE, "None");
        }

    }
    if (options.filter) {
        WsXmlNodeH node = ws_xml_get_child(body, 0 , NULL, NULL);
        WsXmlNodeH filter = ws_xml_add_child(node,
                                XML_NS_WS_MAN, WSENUM_FILTER, options.filter);
        if (options.dialect) {
            ws_xml_add_node_attr(filter, NULL, WSENUM_DIALECT, options.dialect);
        }
    }

    if (strcmp(op, WSENUM_PULL) == 0 ) 
    {
        if (max_elements) {
            WsXmlNodeH node = ws_xml_get_child(body,0 , NULL, NULL);
            ws_xml_add_child_format(node, XML_NS_ENUMERATION,
                                        WSENUM_MAX_ELEMENTS, "%d", max_elements);
        }
    }

    if ( rqstDoc ) 
    {
        if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc));
        }
        respDoc = ws_send_get_response(cl, rqstDoc, options.timeout); 
        if (!respDoc) {
            error( "response doc is null");
        }
        ws_xml_destroy_doc(rqstDoc);
    } else {
        error( "wsman_build_envelope failed");
    }
    return respDoc;
}



static WsManConnection* 
init_client_connection(WsManClientData *cld)
{
   WsManConnection *c=(WsManConnection*)u_zalloc(sizeof(WsManConnection));
   c->response = NULL;
   c->request = NULL;

   return c;
}


void
release_connection(WsManConnection *conn) 
{
	if (conn == NULL)
		return;
		
    if (conn->request) {
		u_free(conn->request);
		conn->request = NULL;
	}
	
    if (conn->response) {
		u_free(conn->response);
		conn->response = NULL;
	}
	/*
	if (conn->response == NULL && conn->request == NULL) {
		u_free(conn);
	}
	*/		
}




WsManClient*
wsman_connect( WsContextH wscntxt,
        const char *hostname,
        const int port,
        const char *path,
        const char *scheme,
        const char *username,
        const char *password)
{
    WsManClient  *wsc     = (WsManClient*)calloc(1, sizeof(WsManClient));
    wsc->hdl              = &wsc->data;
    wsc->wscntx	          = wscntxt;

    wsc->data.hostName    = hostname ? strdup(hostname) : strdup("localhost");
    wsc->data.port        = port;

    wsc->data.user        = username ? strdup(username) : NULL;
    wsc->data.pwd         = password ? strdup(password) : NULL;


    wsc->data.endpoint =  u_strdup_printf("%s://%s:%d%s",
                                     scheme, hostname, port, path);
    debug( "Endpoint: %s", wsc->data.endpoint);

//    wsc->data.scheme      = scheme ? strdup(scheme) : strdup("http");
//    wsc->data.auth_method = 0;
    //wsc->proxyData.proxy = NULL;
    //wsc->proxyData.proxy_auth = NULL;

    //wsc->certData.certFile = certFile ? u_strdup(certFile) : NULL;
    //wsc->certData.keyFile = keyFile ? u_strdup(keyFile) : NULL;
    //wsc->certData.verify_peer = FALSE;

    wsc->connection = init_client_connection(&wsc->data);	

    return wsc;
}



WsManClientStatus 
wsman_release_client(WsManClient * cl)
{
  WsManClientStatus rc={0,NULL}; 

  if (cl->data.hostName) {
    u_free(cl->data.hostName);
  }
  if (cl->data.user) {
    u_free(cl->data.user);
  }
  if (cl->data.pwd) {
    u_free(cl->data.pwd);
  }
  if (cl->data.endpoint) {
    u_free(cl->data.endpoint);
  }

  /*
  if (cl->connection) 
	release_connection(cl->connection);
	*/

  u_free(cl);
  return rc;
}



int
soap_submit_client_op(SoapOpH op, WsManClient *cl)
{    
    env_t *fw = (env_t *)ws_context_get_runtime(cl->wscntx);
    wsman_client(cl, ((op_t*)op)->out_doc);
    char* response = cl->connection->response;
    if (response) 
    {
        WsmanMessage *msg = wsman_soap_message_new();
        char *buf = (char *)u_zalloc(  strlen(response) + 1);

        strncpy (buf, response, strlen(response));
        msg->response.body = u_strdup(buf);
        msg->response.length = strlen(buf);

        WsXmlDocH in_doc =  wsman_build_inbound_envelope(fw, msg);

        u_free(buf);
        wsman_soap_message_destroy(msg);

        ((op_t*)op)->in_doc = in_doc;
    } else {
        ((op_t*)op)->in_doc = NULL;
    }

    return 0;
}

WsXmlDocH
ws_send_get_response(WsManClient *cl,
                     WsXmlDocH rqstDoc,
                     unsigned long timeout)
{
    WsXmlDocH respDoc = NULL;    
    SoapH soap = ws_context_get_runtime(cl->wscntx);

    if ( rqstDoc != NULL && soap != NULL )
    {
        SoapOpH op;
        op = soap_create_op(soap, NULL, NULL, NULL, NULL, NULL, 0, timeout);
        if ( op != NULL ) {
            soap_set_op_doc(op, rqstDoc, 0);
            soap_submit_client_op(op, cl);
            respDoc = soap_detach_op_doc(op, 1);
            soap_destroy_op(op);
        }
    }
    return respDoc;
}

/**
 * Buid Inbound Envelope from Response
 * @param cl Client Handler
 * @return XML document with Envelope
 */
#if 0
WsXmlDocH
wsman_build_envelope_from_response (WsManClient *cl)
{
    WsXmlDocH doc = NULL;
    u_buf_t *buffer;

    env_t *fw = (env_t *)ws_context_get_runtime(cl->wscntx);
    char* response = (char *)u_buf_ptr(cl->connection->response);
    if (response)
    {
        if (u_buf_create(&buffer) != 0) {
            error("Error while creating buffer");
        } else {
            u_buf_set(buffer, response,  strlen(response) );
        }
    }

    if ( (doc = ws_xml_read_memory((SoapH)fw,
                    u_buf_ptr(buffer),
                    u_buf_size(buffer), NULL, 0)) != NULL )
    {
        debug("xml doc received...");
    }
    u_buf_free(buffer);
    return doc;
}

#endif
