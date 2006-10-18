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



struct _WsmanClientHandler {
    WsmanClientFn    	fn;   
    void*  		user_data;
    unsigned int      		id;
};
typedef struct _WsmanClientHandler WsmanClientHandler;

static list_t *handlers;

void 
wsman_add_selector_from_uri( WsXmlDocH doc, 
                             char *resourceUri)
{
    xmlURIPtr uri = NULL;
    WsXmlNodeH header = ws_xml_get_soap_header(doc);

    if (resourceUri != NULL )	
        uri = xmlParseURI((const char *)resourceUri);

    if (uri->query != NULL  ) {
        hash_t * query = parse_query(uri->query, "&");
        hnode_t *hn;
        hscan_t hs;
        hash_scan_begin(&hs, query);
        while ((hn = hash_scan_next(&hs))) {
            wsman_add_selector( header, 
                    (char*) hnode_getkey(hn), 
                    (char*) hnode_get(hn));
            debug("key=%s value=%s", (char*) hnode_getkey(hn) , (char*) hnode_get(hn));
        }
        debug("%d", hash_count(query) );
        hash_free_nodes(query);
        hash_destroy(query);
    }
    if (uri)
        xmlFree(uri);
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


WsXmlDocH transfer_create( WsManClient *cl, char *resourceUri, hash_t *prop, actionOptions options) {
    return NULL;
}

WsXmlDocH 
transfer_get( WsManClient *cl,
              char *resourceUri,
              actionOptions options) 
{
    char *action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);
    char *clean_uri = wsman_remove_query_string(resourceUri);
    WsXmlDocH respDoc = NULL;

    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    WsXmlDocH rqstDoc = wsman_build_envelope(wsc->wscntx, action, WSA_TO_ANONYMOUS, NULL,
            clean_uri , wsc->data.endpoint, options );


    wsman_add_selector_from_uri(rqstDoc, resourceUri);
    if (options.cim_ns)
        wsman_add_selector(ws_xml_get_soap_header(rqstDoc), CIM_NAMESPACE_SELECTOR, options.cim_ns);

    if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc));
    }
    respDoc = ws_send_get_response(cl, rqstDoc, options.timeout);

    ws_xml_destroy_doc(rqstDoc);
    u_free(action);
    u_free(clean_uri);
    return respDoc;

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



WsXmlDocH
transfer_put( WsManClient *cl,
              char *resourceUri,
              hash_t *prop,
              actionOptions options) 
{
    char *action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);
    WsXmlDocH get_respDoc = NULL;
    WsXmlDocH respDoc = NULL;

    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    WsXmlDocH get_rqstDoc = wsman_build_envelope(wsc->wscntx, action, WSA_TO_ANONYMOUS, NULL, 
            wsman_remove_query_string(resourceUri), wsc->data.endpoint, options);
    free(action);
    wsman_add_selector_from_uri( get_rqstDoc, resourceUri);
    get_respDoc = ws_send_get_response(cl, get_rqstDoc, options.timeout);

    WsXmlNodeH get_body = ws_xml_get_soap_body(get_respDoc);

    action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_PUT);
    WsXmlDocH put_rqstDoc = wsman_build_envelope(wsc->wscntx, action, WSA_TO_ANONYMOUS, NULL, wsman_remove_query_string(resourceUri),
            wsc->data.endpoint, options);

    wsman_add_selector_from_uri( put_rqstDoc, resourceUri);
    if (options.cim_ns)
        wsman_add_selector(ws_xml_get_soap_header(put_rqstDoc), CIM_NAMESPACE_SELECTOR, options.cim_ns);

    WsXmlNodeH put_body = ws_xml_get_soap_body(put_rqstDoc);
    ws_xml_duplicate_tree(put_body, ws_xml_get_child(get_body, 0 , NULL, NULL));

    WsXmlNodeH resource_node = ws_xml_get_child(put_body, 0 , NULL, NULL);
    char *nsUri = ws_xml_get_node_name_ns_uri(resource_node);
    hscan_t hs;
    hnode_t *hn;
    hash_scan_begin(&hs, prop);
    while ((hn = hash_scan_next(&hs))) {    	
        WsXmlNodeH n = ws_xml_get_child(resource_node, 0 , nsUri ,(char*) hnode_getkey(hn) );
        ws_xml_set_node_text(n, (char*) hnode_get(hn));
    }

    if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
        ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(put_rqstDoc));
    }
    respDoc = ws_send_get_response(cl, put_rqstDoc, options.timeout);

    ws_xml_destroy_doc(get_rqstDoc);
    ws_xml_destroy_doc(put_rqstDoc);
    free(action);
    return respDoc;

}


WsXmlDocH
invoke( WsManClient *cl,
        char *resourceUri,
        char *method,
        hash_t *prop,
        actionOptions options)
{
    WsXmlDocH respDoc = NULL;

    WsManClientEnc *wsc =(WsManClientEnc*)cl;	

    char *action = NULL;
    char *uri =  wsman_remove_query_string(resourceUri);
    if (strchr(method, '/'))
        action = method;
    else
        action = wsman_make_action(uri , method );
        
    WsXmlDocH rqstDoc = wsman_build_envelope(wsc->wscntx, action, WSA_TO_ANONYMOUS, NULL,
            uri , wsc->data.endpoint, options );

    wsman_add_selector_from_uri( rqstDoc, resourceUri);
    if (options.cim_ns)
        wsman_add_selector(ws_xml_get_soap_header(rqstDoc), CIM_NAMESPACE_SELECTOR, options.cim_ns);
    
    WsXmlNodeH argsin = NULL;
    if (prop)
        argsin = ws_xml_add_empty_child_format(ws_xml_get_soap_body(rqstDoc), uri , "%s_INPUT", method);
    hscan_t hs;
    hnode_t *hn;
    hash_scan_begin(&hs, prop);
    while ((hn = hash_scan_next(&hs))) {    	
        ws_xml_add_child(argsin, NULL, (char*) hnode_getkey(hn) ,  (char*) hnode_get(hn)  );
    }
    
    if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
        if (rqstDoc)
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc));
    }
    respDoc = ws_send_get_response(cl, rqstDoc, options.timeout);
    ws_xml_destroy_doc(rqstDoc);
    if (action) 
        u_free(action);
    u_free(uri);
    return respDoc;

}

WsXmlDocH
wsman_identify( WsManClient *cl,
                actionOptions options)
{
    WsXmlDocH respDoc = NULL;
    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    WsXmlDocH doc = ws_xml_create_envelope(ws_context_get_runtime(wsc->wscntx), NULL);

    ws_xml_add_child(ws_xml_get_soap_body(doc), XML_NS_WSMAN_ID, WSMID_IDENTIFY , NULL);
    // Optional: add vendor specific content extension to node 
        
    respDoc = ws_send_get_response(cl, doc, options.timeout);
    ws_xml_destroy_doc(doc);
    return respDoc;
}


char *wsenum_get_enum_context(WsXmlDocH doc) {
    char* enumContext = NULL;
    WsXmlNodeH enumStartNode = ws_xml_get_child(ws_xml_get_soap_body(doc), 0, NULL, NULL);

    if ( enumStartNode ) {    		
        WsXmlNodeH cntxNode = ws_xml_get_child(enumStartNode, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
        enumContext = u_str_clone(ws_xml_get_node_text(cntxNode));
    } else {
        return NULL;
    }
    return enumContext;
}

WsXmlDocH
wsenum_enumerate( WsManClient* cl,
                  char *resourceUri,
                  int max_elements , 
                  actionOptions options) {
    debug( "Enumerate...");

    WsXmlDocH respDoc = wsman_enum_send_get_response(cl, WSENUM_ENUMERATE, NULL, resourceUri, max_elements, options);
    if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
        if (respDoc)
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(respDoc));
    }
    return respDoc;
}


WsXmlDocH wsenum_pull( WsManClient* cl, char *resourceUri, char *enumContext, int max_elements, actionOptions options)
{
    debug( "Pull... ");
    WsXmlDocH respDoc;
    if ( enumContext || (enumContext && enumContext[0] == 0) ) {
        respDoc = wsman_enum_send_get_response(cl, WSENUM_PULL, enumContext, resourceUri, max_elements, options);
    } else {
        return NULL;
        debug( "No enumeration context ???");
    }
    return respDoc;
}

WsXmlDocH wsenum_release( WsManClient* cl, char *resourceUri, char *enumContext, actionOptions options)  {
    WsXmlDocH respDoc = wsman_enum_send_get_response(cl, WSENUM_RELEASE, enumContext, resourceUri, 0 , options);
    return respDoc;
}

static WsManClientStatus releaseClient(WsManClient * cl)
{
  WsManClientStatus rc={0,NULL};
  WsManClientEnc             * wsc = (WsManClientEnc*)cl;

  if (wsc->data.hostName) {
    free(wsc->data.hostName);
  }
  if (wsc->data.user) {
    free(wsc->data.user);
  }
  if (wsc->data.pwd) {
    free(wsc->data.pwd);
  }
  if (wsc->data.endpoint) {
    free(wsc->data.endpoint);
  }    
  if (wsc->data.scheme) {
    free(wsc->data.scheme);
  }
  if (wsc->certData.certFile) {
    free(wsc->certData.certFile);
  }
  if (wsc->certData.keyFile) {
    free(wsc->certData.keyFile);
  }

  // if (wsc->connection) CMRelease(wsc->connection);

  free(wsc);
  return rc;
}



static WsManClientFT clientFt = {   	
    releaseClient,
    transfer_get, 	
    transfer_put, 	
    wsenum_enumerate,
    wsenum_pull,
    wsenum_release,
    transfer_create,
    invoke,
    wsman_identify
};




WsXmlDocH wsman_make_enum_message(WsContextH soap, char* op, char* enumContext, 
        char* resourceUri, char* url, actionOptions options) {
    char* action = wsman_make_action(XML_NS_ENUMERATION, op);
    WsXmlDocH doc = wsman_build_envelope(soap, action, WSA_TO_ANONYMOUS, NULL, resourceUri, url, options );
            
    if ( doc != NULL ) {
        WsXmlNodeH node = ws_xml_add_child(ws_xml_get_soap_body(doc), XML_NS_ENUMERATION, op, NULL);
        if ( enumContext ) {
            ws_xml_add_child(node, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, enumContext);
        }
    }
    free(action);
    return doc;
}



WsXmlDocH wsman_enum_send_get_response(WsManClient *cl, char* op, char* enumContext, char* resourceUri, 
        int max_elements, actionOptions options)
{
    WsXmlDocH respDoc  = NULL;
    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    WsXmlDocH rqstDoc = wsman_make_enum_message( wsc->wscntx, op, enumContext, resourceUri, wsc->data.endpoint, options);

    if (options.cim_ns)
        wsman_add_selector(ws_xml_get_soap_header(rqstDoc), CIM_NAMESPACE_SELECTOR, options.cim_ns);

    if ((options.flags & FLAG_ENUMERATION_COUNT_ESTIMATION) == FLAG_ENUMERATION_COUNT_ESTIMATION) {
        WsXmlNodeH header = ws_xml_get_soap_header(rqstDoc);
        ws_xml_add_child(header, XML_NS_WS_MAN, WSM_REQUEST_TOTAL, NULL);
    }

    if (strcmp(op, WSENUM_ENUMERATE) == 0 ) {
        WsXmlNodeH node = ws_xml_get_child(ws_xml_get_soap_body(rqstDoc), 0 , NULL, NULL);
        if ((options.flags & FLAG_ENUMERATION_OPTIMIZATION) == FLAG_ENUMERATION_OPTIMIZATION) {
            ws_xml_add_child(node, XML_NS_WS_MAN, WSM_OPTIMIZE_ENUM, NULL);
            if (max_elements) {
                ws_xml_add_child_format(node , XML_NS_WS_MAN, WSENUM_MAX_ELEMENTS, "%d", max_elements);
            }
        }
        if ((options.flags & FLAG_ENUMERATION_ENUM_EPR) == FLAG_ENUMERATION_ENUM_EPR) 
        {
            ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE, WSM_ENUM_EPR);
        } 
        else if ((options.flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) == FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) 
        {
            ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE, WSM_ENUM_OBJ_AND_EPR);
        }

        if ((options.flags & FLAG_IncludeSubClassProperties) == FLAG_IncludeSubClassProperties) 
        {
            ws_xml_add_child(node, XML_NS_CIM_BINDING, WSMB_POLYMORPHISM_MODE, WSMB_INCLUDE_SUBCLASS_PROP);
        } 
        else if ((options.flags & FLAG_ExcludeSubClassProperties) == FLAG_ExcludeSubClassProperties) 
        {
            ws_xml_add_child(node, XML_NS_CIM_BINDING, WSMB_POLYMORPHISM_MODE, WSMB_EXCLUDE_SUBCLASS_PROP);
        }
        else if ((options.flags & FLAG_POLYMORPHISM_NONE) == FLAG_POLYMORPHISM_NONE) 
        {
            ws_xml_add_child(node, XML_NS_CIM_BINDING, WSMB_POLYMORPHISM_MODE, "None");
        }

    }
    if (options.filter)
    {
        WsXmlNodeH node = ws_xml_get_child(ws_xml_get_soap_body(rqstDoc), 0 , NULL, NULL);
        WsXmlNodeH filter = ws_xml_add_child(node, XML_NS_WS_MAN, WSENUM_FILTER, options.filter);
        if (options.dialect) {
            ws_xml_add_node_attr(filter, NULL, WSENUM_DIALECT, options.dialect);
        }
    }

    if (strcmp(op, WSENUM_PULL) == 0 ) {
        if (max_elements) {
            WsXmlNodeH node = ws_xml_get_child(ws_xml_get_soap_body(rqstDoc), 0 , NULL, NULL);
            ws_xml_add_child_format(node, XML_NS_ENUMERATION, WSENUM_MAX_ELEMENTS, "%d", max_elements);
        }
    }

    if ( rqstDoc ) {       
        if ((options.flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc));
        }
        respDoc = ws_send_get_response(cl, rqstDoc, options.timeout); 
        if (!respDoc) {
            debug( "response doc is null");
        }
        ws_xml_destroy_doc(rqstDoc);
    } else {
        debug( "wsman_build_envelope failed");
    }
    return respDoc;
}



WsManConnection *initClientConnection(WsManClientData *cld)
{
   WsManConnection *c=(WsManConnection*)calloc(1,sizeof(WsManConnection));
   c->response = NULL;
   c->request = NULL;

   return c;
}

WsManClient *wsman_connect( 
		WsContextH wscntxt,
		const char *hostname,
		const int port,
		const char *path,
		const char *scheme,
		const char *username,
		const char *password,
		WsManClientStatus *rc)
{
    return wsman_connect_with_ssl(wscntxt, hostname, port, path, scheme, username, password, NULL, NULL, rc);
}

WsManClient *wsman_connect_with_ssl( 
		WsContextH wscntxt,
		const char *hostname,
		const int port,
		const char *path,
		const char *scheme,
		const char *username,
		const char *password,
                const char * certFile, 
                const char * keyFile,
		WsManClientStatus *rc)
{
    WsManClientEnc *wsc = (WsManClientEnc*)calloc(1, sizeof(WsManClientEnc));
    wsc->enc.hdl          = &wsc->data;
    wsc->enc.ft           = &clientFt;

    wsc->wscntx			  = wscntxt;

    wsc->data.hostName    = hostname ? strdup(hostname) : strdup("localhost");
    wsc->data.user        = username ? strdup(username) : NULL;
    wsc->data.pwd         = password ? strdup(password) : NULL;
    wsc->data.scheme      = scheme ? strdup(scheme) : strdup("http");

    if (port)
        wsc->data.port = port;
    else
        wsc->data.port = strcmp(wsc->data.scheme, "https") == 0 ?  8888 : 8889;

    if (path)
        wsc->data.endpoint =  u_strdup_printf("%s://%s:%d%s", wsc->data.scheme  , hostname, port, path);
    else
        wsc->data.endpoint =  u_strdup_printf("%s://%s:%d/wsman", wsc->data.scheme  , hostname, port);

    debug( "Endpoint: %s", wsc->data.endpoint);
    wsc->certData.certFile = certFile ? strdup(certFile) : NULL;
    wsc->certData.keyFile = keyFile ? strdup(keyFile) : NULL;

    wsc->connection=initClientConnection(&wsc->data);	
    return (WsManClient *)wsc;
}


unsigned int
wsman_client_add_handler (
        WsmanClientFn    fn,                     
        void     *user_data)
{
    WsmanClientHandler *handler;

    assert (fn);

    handler = u_zalloc (sizeof(WsmanClientHandler));

    handler->fn = fn;
    handler->user_data = user_data;

    if (handlers != NULL) {
        
        lnode_t *node = list_last(handlers);
        handler->id = ((WsmanClientHandler *) node->list_data)->id + 1;
    } else {
        handlers = list_create(LISTCOUNT_T_MAX);
        handler->id = 1;
    }

    lnode_t *h = lnode_create(handler);
    list_prepend (handlers, h);

    return handler->id;
}

void
wsman_client_remove_handler (unsigned int id)
{
    lnode_t *iter = list_first(handlers);
    while (iter != NULL) {
        WsmanClientHandler *handler = (WsmanClientHandler *)iter->list_data;

        if (handler->id == id) 
        {
            list_delete (handlers, iter);
            u_free (handler);
            return;
        }
        iter = list_next(handlers, iter);
    }
}

static long long transfer_time = 0;
long long
get_transfer_time()
{
    return transfer_time;
}

void
wsman_client (WsManClient *cl, 
	WsXmlDocH rqstDoc)
{    	
    lnode_t *iter = list_first(handlers);
    struct timeval tv0, tv1;
    long long t0, t1;
    while (iter) 
    {
        WsmanClientHandler *handler = (WsmanClientHandler *)iter->list_data;
        gettimeofday(&tv0, NULL);
       	handler->fn (cl, rqstDoc, handler->user_data);
        gettimeofday(&tv1, NULL);
        t0 = tv0.tv_sec * 10000000 + tv0.tv_usec;
        t1 = tv1.tv_sec * 10000000 + tv1.tv_usec;
        transfer_time += t1 -t0;
        iter = list_next(handlers, iter);
    }     
    return;
}

int soap_submit_client_op(SoapOpH op, WsManClient *cl )
{	
    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    env_t *fw = (env_t *)ws_context_get_runtime(wsc->wscntx);  
    wsman_client(cl, ((op_t*)op)->out_doc);
    char* response = wsc->connection->response;
    if (response) {
        char *buf = (char *)u_zalloc(  strlen(response) + 1);
        strncpy (buf, response, strlen(response));
        WsmanMessage *msg = wsman_soap_message_new();
        msg->response.body = strdup(buf);
        msg->response.length = strlen(buf);
        WsXmlDocH in_doc =  wsman_build_inbound_envelope(fw, msg);	
        free(buf);
        ((op_t*)op)->in_doc = in_doc;
    } else {
        ((op_t*)op)->in_doc = NULL;
    }

    return 0;
}

WsXmlDocH ws_send_get_response(WsManClient *cl, WsXmlDocH rqstDoc, unsigned long timeout)
{
    WsXmlDocH respDoc = NULL;
    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    SoapH soap = ws_context_get_runtime(wsc->wscntx);

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



