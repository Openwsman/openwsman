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


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>


#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "ws_dispatcher.h"

#include "xml_api_generic.h"
#include "xml_serializer.h" 
#include "wsman-faults.h"
#include "wsman-debug.h"
#include "wsman-client.h"



struct _WsmanClientHandler {
    WsmanClientFn    	fn;   
    gpointer     		user_data;
    guint        		id;
};
typedef struct _WsmanClientHandler WsmanClientHandler;

static GSList *handlers = NULL;

char* wsman_add_selector_from_uri(
        WsManClient *cl, 
        WsXmlDocH doc, 
        char *resourceUri)
{
    int j;
    const char *q = NULL;
    struct pair_t *query = NULL;

    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    if (resourceUri != NULL )	
        q = strchr(resourceUri, '?');		

    if (!q)
        return resourceUri;
    else 
    {
        query = parse_query(q+1, '&');
        if ( doc )
        {
            if (query != NULL) 
            {
                for(j = 0; query[j].name; j++)
                {
                    wsman_set_selector(wsc->wscntx, doc, query[j].name, query[j].value);
                }
            }                       		              
        }		
        // Return Resource Uri without query string.
        int len =  q - resourceUri;
        char *res = (char *)malloc(len+1);
        strncpy( res, resourceUri,  len);    		
        return res;
    }		
}


char* wsman_make_action(char* uri, char* opName) {
    int len = strlen(uri) + strlen(opName) + 2;
    char* ptr = (char*)malloc(len);
    if ( ptr )
        sprintf(ptr, "%s/%s", uri, opName);
    return ptr;
}



WsXmlDocH transfer_create( WsManClient *cl, char *resourceUri, GList *prop) {   		      
    return NULL;
}


WsXmlDocH transfer_put(WsManClient *cl, char *resourceUri, GList *prop) {   		      
    char *action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);
    WsXmlDocH get_respDoc = NULL;
    WsXmlDocH respDoc = NULL;

    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    WsXmlDocH get_rqstDoc = wsman_build_envelope(wsc->wscntx, action,
            WSA_TO_ANONYMOUS, NULL, wsman_remove_query_string(resourceUri), wsc->data.endpoint,
            60000, 50000);	

    wsman_add_selector_from_uri(cl, get_rqstDoc, resourceUri);
    get_respDoc = ws_send_get_response(cl, get_rqstDoc, 60000);

    WsXmlNodeH get_body = ws_xml_get_soap_body(get_respDoc);

    free(action);
    action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_PUT);
    WsXmlDocH put_rqstDoc = wsman_build_envelope(wsc->wscntx,
            action, WSA_TO_ANONYMOUS, NULL, wsman_remove_query_string(resourceUri),
            wsc->data.endpoint, 60000, 50000);	

    wsman_add_selector_from_uri(cl, put_rqstDoc, resourceUri);
    WsXmlNodeH put_body = ws_xml_get_soap_body(put_rqstDoc);
    ws_xml_duplicate_tree(put_body, ws_xml_get_child(get_body, 0 , NULL, NULL));

    GList * node = prop;
    
    WsXmlNodeH resource_node = ws_xml_get_child(put_body, 0 , NULL, NULL);
    char *nsUri = ws_xml_get_node_name_ns_uri(resource_node);
    while(node) 
    {
        WsProperties *p = (WsProperties *)node->data;
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "key= %s, value= %s", p->key, p->value);
        WsXmlNodeH n = ws_xml_get_child(resource_node, 0 , nsUri , p->key );
        ws_xml_set_node_text(n, p->value);
        node = g_list_next (node);
    }
    
    respDoc = ws_send_get_response(cl, put_rqstDoc, 60000);

    ws_xml_destroy_doc(get_rqstDoc);
    ws_xml_destroy_doc(put_rqstDoc);
    free(action);
    return respDoc;

}


WsXmlDocH invoke(WsManClient *cl, char *resourceUri , char *method, GList *prop) {
    WsXmlDocH respDoc = NULL;

    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "ResourceUri= %s", wsman_remove_query_string(resourceUri));

    char *action = NULL;
    char *uri =  wsman_remove_query_string(resourceUri);
    if (strchr(method, '/'))
        action = method;
    else
        action = wsman_make_action(uri , method );
        
    WsXmlDocH rqstDoc = wsman_build_envelope(wsc->wscntx, action, WSA_TO_ANONYMOUS, NULL,
            uri , wsc->data.endpoint, 60000, 50000);	

    wsman_add_selector_from_uri(cl, rqstDoc, resourceUri);

    GList * node = prop;
    
    WsXmlNodeH argsin;
    if (prop)
        argsin = ws_xml_add_empty_child_format(ws_xml_get_soap_body(rqstDoc), uri , "%s_INPUT", method);
    while(node) 
    {
        WsProperties *p = (WsProperties *)node->data;
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "key= %s, value= %s", p->key, p->value);
        ws_xml_add_child(argsin, NULL, p->key , p->value );
        node = g_list_next (node);
    }
    
    ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc));
    respDoc = ws_send_get_response(cl, rqstDoc, 60000);
    ws_xml_destroy_doc(rqstDoc);
    if (action) free(action);
    return respDoc;

}


WsXmlDocH transfer_get(WsManClient *cl, char *resourceUri) {   		      
    char *action = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);
    WsXmlDocH respDoc = NULL;

    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    WsXmlDocH rqstDoc = wsman_build_envelope(wsc->wscntx, action, WSA_TO_ANONYMOUS, NULL,
            wsman_remove_query_string(resourceUri), wsc->data.endpoint, 60000, 50000);	

    wsman_add_selector_from_uri(cl, rqstDoc, resourceUri);

    respDoc = ws_send_get_response(cl, rqstDoc, 60000);
    ws_xml_destroy_doc(rqstDoc);
    free(action);
    return respDoc;

}



GList *enumerate( WsManClient* cl, char *resourceUri, int count) {
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Doing enumeration");
    char* enumContext = NULL;
    GList *enumeration = NULL;

    WsXmlDocH respDoc = wsman_enum_send_get_response(cl, WSENUM_ENUMERATE, NULL, resourceUri);   
    WsXmlNodeH enumStartNode = ws_xml_get_child(ws_xml_get_soap_body(respDoc), 0, NULL, NULL);

    int get_all = 0;
    if (count == -1 )
        get_all = 1;

    if ( enumStartNode )
    {    		
        WsXmlNodeH cntxNode = ws_xml_get_child(enumStartNode, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
        enumContext = soap_clone_string(ws_xml_get_node_text(cntxNode));
        if ( enumContext || (enumContext && enumContext[0] == 0) ) {
            while( count && (respDoc = wsman_enum_send_get_response(cl, WSENUM_PULL, enumContext, resourceUri)) ) {

                WsXmlNodeH node = ws_xml_get_child(ws_xml_get_soap_body(respDoc), 0, NULL, NULL);
                if ( strcmp(ws_xml_get_node_local_name(node), WSENUM_PULL_RESP) != 0 ) {                		                		
                    //ws_xml_destroy_doc(ws_xml_get_node_doc(node));
                    break;
                }
                enumeration = g_list_append(enumeration, respDoc);

                if ( ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE) ) {                		
                    //ws_xml_destroy_doc(ws_xml_get_node_doc(node));
                    break;
                }
                if ( (cntxNode = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT)) ) {
                    soap_free(enumContext);
                    enumContext = soap_clone_string(ws_xml_get_node_text(cntxNode));
                }                				

                //ws_xml_destroy_doc(ws_xml_get_node_doc(node));
                count--;
                if ( enumContext == NULL || enumContext[0] == 0 ) {
                    wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "No enumeration context");
                    break;
                }
            }
            if ( !count  && !get_all) {
                respDoc = wsman_enum_send_get_response(cl, 
                        WSENUM_RELEASE, 
                        enumContext, 
                        resourceUri);
                //ws_xml_destroy_doc(ws_xml_get_node_doc(node));
            }
        }
        else
        {
            if (respDoc)
                enumeration = g_list_append(enumeration, respDoc);
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "No enumeration context ???");
        }
        //ws_xml_destroy_doc(ws_xml_get_node_doc(enumStartNode));
    } else {    		
        enumeration = NULL;
    }
    return enumeration;
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
    enumerate,
    transfer_create,
    invoke
};




WsXmlDocH wsman_make_enum_message(WsContextH soap, 
        char* op, 
        char* enumContext,
        char* resourceUri,
        char* url)
{
    char* action = wsman_make_action(XML_NS_ENUMERATION, op);
    WsXmlDocH doc = wsman_build_envelope(soap, 
            action, 
            WSA_TO_ANONYMOUS, 
            NULL, 
            resourceUri,
            url,
            60000,
            50000); 
            
    if ( doc != NULL )
    {
        WsXmlNodeH node = ws_xml_add_child(ws_xml_get_soap_body(doc), 
                XML_NS_ENUMERATION, 
                op, 
                NULL);
        if ( enumContext ) 
        {
            ws_xml_add_child(node, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, enumContext);
        }
    }

    free(action);
    return doc;
}



WsXmlDocH wsman_enum_send_get_response(WsManClient *cl, 
        char* op, 
        char* enumContext, 
        char* resourceUri)
{
    WsXmlDocH respDoc  = NULL;
    WsManClientEnc *wsc =(WsManClientEnc*)cl;	
    WsXmlDocH rqstDoc = wsman_make_enum_message(
    			wsc->wscntx, 
            op, 
            enumContext,
            resourceUri,
            wsc->data.endpoint);

    if ( rqstDoc )
    {       
        respDoc = ws_send_get_response(cl, rqstDoc, 60000); 
        if (!respDoc)
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "response doc is null");
        ws_xml_destroy_doc(rqstDoc);
    }
    else 
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "wsman_build_envelope failed");
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
		const char *scheme,
		const char *username,
		const char *password,
		WsManClientStatus *rc)
{
    return wsman_connect_with_ssl(wscntxt, hostname, port, scheme, username, password, NULL, NULL, rc);
}

WsManClient *wsman_connect_with_ssl( 
		WsContextH wscntxt,
		const char *hostname,
		const int port,
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

    wsc->data.endpoint =  g_strdup_printf("%s://%s:%d/%s", wsc->data.scheme  , hostname, port, "wsman");
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Endpoint: %s", wsc->data.endpoint);
    wsc->certData.certFile = certFile ? strdup(certFile) : NULL;
    wsc->certData.keyFile = keyFile ? strdup(keyFile) : NULL;

    wsc->connection=initClientConnection(&wsc->data);	
    return (WsManClient *)wsc;
}



guint
wsman_client_add_handler (WsmanClientFn    fn,                     
                      gpointer     user_data)
{
    WsmanClientHandler *handler;

    g_assert (fn);

    handler = g_new0 (WsmanClientHandler, 1);

    handler->fn = fn;    
    handler->user_data = user_data;

    if (handlers)
        handler->id = ((WsmanClientHandler *) handlers->data)->id + 1;
    else
        handler->id = 1;

    handlers = g_slist_prepend (handlers, handler);

    return handler->id;
}

void
wsman_client_remove_handler (guint id)
{
    GSList *iter;

    iter = handlers;
    while (iter) {
        WsmanClientHandler *handler = (WsmanClientHandler *)iter->data;

        if (handler->id == id) 
        {
            handlers = g_slist_remove_link (handlers, iter);
            g_free (handler);
            return;
        }

        iter = iter->next;
    }

    wsman_debug (WSMAN_DEBUG_LEVEL_WARNING, "Couldn't find client handler %d", id);
}

void
wsman_client (WsManClient *cl, 
				WsXmlDocH rqstDoc
                )
{    	
    GSList *iter;      
    iter = handlers;
    // FIXME: Only one handler available.
    while (iter) 
    {
        WsmanClientHandler *handler = (WsmanClientHandler *)iter->data;       
       	handler->fn (cl, rqstDoc, handler->user_data);       	       	
        iter = iter->next;
    }     
    return;
}


