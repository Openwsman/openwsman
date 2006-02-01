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
 
#include "config.h"
 
 
#include <stdlib.h> 
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>
#include <gmodule.h>



#include <libsoup/soup-address.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-server.h>
#include <libsoup/soup-server-auth.h>
#include <libsoup/soup-server-message.h>


#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "soap_api.h"

#include "xml_api_generic.h"
#include "xml_serializer.h"
#include "ws_dispatcher.h"


#include "wsman-debug.h"
#include "ws_transport.h"
#include "wsmand-listener.h"
#include "wsmand-plugins.h"
#include "wsmand-daemon.h"



static void
print_header (gpointer name, gpointer value, gpointer data)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "%s: %s", (char *)name, (char *)value);
}


static gboolean
server_auth_callback (
		SoupServerAuthContext *auth_ctx, 
		SoupServerAuth        *auth,
		SoupMessage           *msg,
		gpointer               data) 
{
	const char *username;
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Authenticating...");
	soup_message_foreach_header (msg->request_headers, print_header, NULL);
	
	soup_message_add_header (msg->response_headers, "Server", PACKAGE"/"VERSION );
    if (auth) 
    {
		username = soup_server_auth_get_user (auth);		
		if ( !strcmp(username, "wsman") 
			&& soup_server_auth_check_passwd(auth, "secret" ) )
			return TRUE;		 
			
    } 
    			
	soup_message_add_header (msg->response_headers, "Content-Length", "0" );    			
	soup_message_add_header (msg->response_headers, "Connection", "close" );  
	return FALSE;
}




static void
server_callback (SoupServerContext *context, SoupMessage *msg, gpointer data)
{		
    char *path;
       
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"Server Callback Called\n");
    path = soup_uri_to_string (soup_message_get_uri (msg), TRUE);
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"%s %s HTTP/1.%d", msg->method, path,
            soup_message_get_http_version (msg));
            
            
    soup_message_foreach_header (msg->request_headers, print_header, NULL);
    
    
    if (msg->request.length)
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"Request: %.*s", msg->request.length, msg->request.body);
    }
    
    
  	if (soup_method_get_id (msg->method) != SOUP_METHOD_ID_POST)
  	{
        soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
        goto DONE;
    }   

    if (path) 
    {
        if (strcmp(path, "/wsman"))\
        {
            soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
            goto DONE;
        }
    } else {
        path = g_strdup ("");
    }    
    
    SOAP_FW* fw = (SOAP_FW*)data;	
	SOAP_CHANNEL* ch;
	ch = make_soap_channel(
		soap_fw_make_unique_id(fw),
		NULL,
		dispatch_inbound_call,
		(SOAP_FW *)data,
		SOAP_CHANNEL_DEF_INACTIVITY_TIMEOUT,
		SOAP_CHANNEL_DEF_KEEP_ALIVE_TIMEOUT,
		SOAP_CHANNEL_DEF_TCP_LISTENER_FLAGS
	);
			
	DL_AddTail(&fw->channelList, &ch->node);
	ch->inputBuffer->bufSize = SOUP_MESSAGE (msg)->request.length;
	ch->inputBuffer->buf = SOUP_MESSAGE (msg)->request.body;	
	
	
	if ( ch->recvDispatchCallback )
	{
    		ch->recvDispatchCallback(ch);
	}
	
	    	
 	if (ch->FaultCodeType != WSMAN_FAULT_NONE )
 	{
 		char *buf;
   		int  len;    		
   		
    		wsman_generate_fault_buffer(fw->cntx, soap_get_ch_doc(ch, fw), 
    			ch->FaultCodeType , 
    			ch->FaultDetailType,
    			&buf, 
    			&len);
    		
		msg->response.owner = SOUP_BUFFER_SYSTEM_OWNED;
		msg->response.length = len;
		msg->response.body = g_malloc (len);
		msg->response.body = buf;   
		soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);        		
 	} 
 	else 
 	{		 	
		int bDone = 0;
		DL_Node* node = DL_GetHead(&ch->outputList);
    		while(node && !bDone)
    		{
    			DL_Node* nextNode = DL_GetNext(node);
        		SOAP_OUTPUT_CHAIN* chain = (SOAP_OUTPUT_CHAIN*)node;	
        
        		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, 
        			"Response: %s", chain->dataBuffer->dataBuf );

    			msg->response.owner = SOUP_BUFFER_SYSTEM_OWNED;
    			msg->response.length = chain->dataBuffer->size;
    			msg->response.body = g_malloc (chain->dataBuffer->size);
    			msg->response.body = chain->dataBuffer->dataBuf;        
        		node = nextNode;
    		}
    		soup_message_set_status (msg, SOUP_STATUS_OK);
 	}
    	

DONE:
    g_free (path);
    soup_server_message_set_encoding (SOUP_SERVER_MESSAGE (msg),
            SOUP_TRANSFER_CONTENT_LENGTH);
	        
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, 
    		"Response (status) %d %s", msg->status_code, msg->reason_phrase);
        
}


WsContextH wsmand_start_listener(WsManListenerH *listener)
{	
	GList *list = NULL;
    WsContextH cntx = NULL;	  	

	if (!wsmand_options_get_no_plugins_flag())
		wsman_plugins_load(listener);

	GList * node = listener->plugins;	
	
	while (node) 
	{		
		WsManPlugin *p = (WsManPlugin *)node->data;		
		p->interface = (WsDispatchInterfaceInfo *)malloc(sizeof(WsDispatchInterfaceInfo));
		
		g_module_symbol(p->p_handle, "get_endpoints", (gpointer*)&p->get_endpoints);
		p->get_endpoints(p->p_handle, p->interface );				
		
		g_return_val_if_fail(p->interface != NULL , NULL );
		
		list = g_list_append(list, p->interface);			
		node = g_list_next (node);
	}
				
    cntx = ws_create_runtime(list);                    	
    return cntx;
}



int wsmand_start_server() 
{	
	SoupServer *server, *ssl_server;
  	// Authentication handler   	   	
   	SoupServerAuthContext auth_ctx = { 0 };	
   	
    server = soup_server_new (SOUP_SERVER_PORT, wsmand_options_get_server_port(), NULL);
    if (!server) 
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
        		"Unable to bind to server port %d", 
        	wsmand_options_get_server_port());
        return 1;
    }		
    
	WsManListenerH *listener = (WsManListenerH *)g_malloc0(sizeof(WsManListenerH) );	
    WsContextH cntx = wsmand_start_listener(listener);           
    g_return_val_if_fail(cntx != NULL, 1 );            
   	SoapH soap = ws_context_get_runtime(cntx);   	
   	
   	
    	
   	// By default, start with basic auth
  
   	char *atype = wsmand_options_get_auth_type();

   	if (!strcmp(atype, "basic"))
   	{
    		auth_ctx.types |= SOUP_AUTH_TYPE_BASIC;
    		auth_ctx.basic_info.realm = AUTHENTICATION_REALM; 
   	}
	else if (!strcmp(atype, "digest")) 
	{
    		auth_ctx.types |= SOUP_AUTH_TYPE_DIGEST;
    		auth_ctx.digest_info.realm = AUTHENTICATION_REALM;
    		auth_ctx.digest_info.allow_algorithms = SOUP_ALGORITHM_MD5;
    		auth_ctx.digest_info.force_integrity = FALSE;		
	}
       	
   	  	
   	
   	auth_ctx.callback = server_auth_callback;   	   	
   	// The server handler to deal with all requests
    soup_server_add_handler (server, NULL, &auth_ctx, server_callback, NULL, (SOAP_FW *)soap);       	    
     
    wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,"Starting Server on port %d",  soup_server_get_port (server));
    soup_server_run_async (server);

    if (wsmand_options_get_ssl_key_file() && wsmand_options_get_ssl_cert_file()) 
    {
        ssl_server = soup_server_new (
                SOUP_SERVER_PORT, wsmand_options_get_server_ssl_port(),
                SOUP_SERVER_SSL_CERT_FILE, wsmand_options_get_ssl_cert_file(),
                SOUP_SERVER_SSL_KEY_FILE, wsmand_options_get_ssl_key_file(),
                NULL);

        if (!ssl_server) 
        {
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,"Unable to bind to SSL server port %d", wsmand_options_get_server_ssl_port());
            exit (1);
        }
        soup_server_add_handler (ssl_server, NULL, NULL,
                server_callback, NULL, NULL);
                
        wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,"Starting SSL Server on port %d", 
                soup_server_get_port (ssl_server));
                
        soup_server_run_async (ssl_server);
    }
    
	wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,"Waiting for requests...");
	return 0;
}



