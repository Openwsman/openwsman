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

#define _GNU_SOURCE
#include "wsman_config.h"


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


#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
extern void start_event_source(SoapH soap);

#include "wsman-xml.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"


#include "wsmand-listener.h"
#include "wsmand-daemon.h"
#include "wsmand-auth.h"
#include "wsman-plugins.h"
#include "wsman-server.h"


int facility = LOG_DAEMON;


static void
print_header (gpointer name, gpointer value, gpointer data)
{
    debug( "%s: %s", (char *)name, (char *)value);
}


static gboolean
server_auth_callback ( SoupServerAuthContext *auth_ctx, SoupServerAuth *auth,
        SoupMessage  *msg, gpointer data) 
{
    char *filename = NULL;
    soup_message_foreach_header (msg->request_headers, print_header, NULL);
    soup_message_add_header (msg->response_headers, "Server", PACKAGE"/"VERSION );
    soup_message_add_header (msg->response_headers, "Content-Type", "application/soap+xml;charset=UTF-8"); 

#if 0 
    WsXmlDocH in_doc = wsman_build_inbound_envelope( (env_t *)data, msg);
    if (wsman_is_identify_request(in_doc)) {
        wsman_create_identify_response( (env_t *)data, msg);
        debug( "Skipping authentication...");
        return TRUE;
    }
#endif

    debug( "Authenticating...");
    if (auth) {
        switch (auth->type) {
        case SOUP_AUTH_TYPE_BASIC:
            filename = wsmand_options_get_basic_password_file();
            break;
        case SOUP_AUTH_TYPE_DIGEST:
            filename = wsmand_options_get_digest_password_file();
            break;
        }
        if (filename) {
            if ( authorize_from_file(auth, filename ))
                return TRUE;		 
        }
    } 

    soup_message_add_header (msg->response_headers, "Content-Length", "0" );    			
    soup_message_add_header (msg->response_headers, "Connection", "close" );  
    return FALSE;
}

static void server_callback (SoupServerContext *context, SoupMessage *msg, 
        gpointer data) {		

    char *path, *default_path;
    char *content_type;
    char *encoding;

    WsmanMessage *wsman_msg = wsman_soap_message_new();

    // Check HTTP headers
    path = soup_uri_to_string (soup_message_get_uri (msg), TRUE);
    debug("%s %s HTTP/1.%d", msg->method, path,
            soup_message_get_http_version (msg));

    soup_message_foreach_header (msg->request_headers, print_header, NULL);
    if (msg->request.length) {
        debug("Request: %.*s", msg->request.length, msg->request.body);
    }

    // Check Method
    if (soup_method_get_id (msg->method) != SOUP_METHOD_ID_POST) {
        soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
        goto DONE;
    }   

    default_path = wsmand_options_get_service_path();
    if (path) {
        if (strcmp(path, "/wsman") != 0 ) {
            soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
            goto DONE;
        }
    } else {
        path = g_strdup ("");
    }    

    content_type = (char *)soup_message_get_header (msg->request_headers, "Content-Type");
    if (content_type && strncmp(content_type, SOAP_CONTENT_TYPE, strlen(SOAP_CONTENT_TYPE)) != 0 ) {
        soup_message_set_status (msg, SOUP_STATUS_BAD_REQUEST);
        goto DONE;
    } else {
        encoding = strchr(content_type, '=') + 1;
        debug("Encoding: %s", encoding);
    }

    env_t* fw = (env_t*)data;	
    wsman_msg->status.fault_code = WSMAN_RC_OK;

    wsman_msg->request.body = (char *)msg->request.body;
    wsman_msg->request.length = msg->request.length;
    /*
    wsman_msg->auth_data.username = soup_server_auth_get_user(context->auth);
    wsman_msg->auth_data.password = context->auth->basic.passwd;
    */

    // Call dispatcher
    dispatch_inbound_call(fw, wsman_msg);
    
    if ( wsman_fault_occured(wsman_msg) ) {
        char *buf;
        int  len;    		
        if (wsman_msg->in_doc != NULL) {
            wsman_generate_fault_buffer(
                    fw->cntx, 
                    wsman_msg->in_doc, 
                    wsman_msg->status.fault_code , 
                    wsman_msg->status.fault_detail_code, 
                    wsman_msg->status.fault_msg, 
                    &buf, &len);
            msg->response.length = len;
            msg->response.body = strndup(buf, len);   
            free(buf);
        } else {
            msg->response.length = 0;
            msg->response.body = NULL;
        }

        msg->response.owner = SOUP_BUFFER_SYSTEM_OWNED;
        soup_message_set_status (msg, SOUP_STATUS_INTERNAL_SERVER_ERROR);        		
        goto DONE;
    } else {		 	
    	msg->response.owner = SOUP_BUFFER_SYSTEM_OWNED;
    	msg->response.length = wsman_msg->response.length;
    	msg->response.body = (char *)wsman_msg->response.body;
        debug("Response: %s", (char *)wsman_msg->response.body);
        soup_message_set_status (msg, wsman_msg->http_code);
    }



    if (wsman_msg->in_doc != NULL) {
        ws_xml_destroy_doc(wsman_msg->in_doc);
    }
DONE:
    g_free (path);
    soup_server_message_set_encoding (SOUP_SERVER_MESSAGE (msg),
            SOUP_TRANSFER_CONTENT_LENGTH);

    u_free(wsman_msg);
    debug( "Response (status) %d %s", msg->status_code, msg->reason_phrase);

}



int wsmand_start_server() 
{	
    SoupServer *server = NULL;
    // Authentication handler   	   	
    SoupServerAuthContext auth_ctx = { 0 };	

    WsManListenerH *listener = wsman_dispatch_list_new();
    WsContextH cntx = wsman_init_plugins(listener);           
    g_return_val_if_fail(cntx != NULL, 1 );            

    SoapH soap = ws_context_get_runtime(cntx);   	

    if (!wsmand_options_get_digest()) {
        auth_ctx.types |= SOUP_AUTH_TYPE_BASIC;
        auth_ctx.basic_info.realm = AUTHENTICATION_REALM; 
    } else {
        auth_ctx.types |= SOUP_AUTH_TYPE_DIGEST;
        auth_ctx.digest_info.realm = AUTHENTICATION_REALM;
        auth_ctx.digest_info.allow_algorithms = SOUP_ALGORITHM_MD5;
        auth_ctx.digest_info.force_integrity = FALSE;		
    }
    auth_ctx.callback = server_auth_callback;   	   	

    // The server handler to deal with all requests
    if (wsmand_options_get_server_port() > 0) 
    {
        server = soup_server_new (SOUP_SERVER_PORT, wsmand_options_get_server_port(), NULL);
        if (!server) {
            debug( "Unable to bind to server port %d", wsmand_options_get_server_port());
            return 1;
        }		
        soup_server_add_handler (server, NULL, &auth_ctx, server_callback, NULL, (env_t *)soap);       	    
        debug("Starting Server on port %d",  soup_server_get_port (server));
        soup_server_run_async (server);
        g_object_unref (server);
    }

#ifdef HAVE_SSL
    SoupServer *ssl_server = NULL;
    if (wsmand_options_get_ssl_key_file() && wsmand_options_get_ssl_cert_file() && wsmand_options_get_server_ssl_port() > 0)  
    {
        ssl_server = soup_server_new (
                SOUP_SERVER_PORT, wsmand_options_get_server_ssl_port(),
                SOUP_SERVER_SSL_CERT_FILE, wsmand_options_get_ssl_cert_file(),
                SOUP_SERVER_SSL_KEY_FILE, wsmand_options_get_ssl_key_file(),
                NULL);

        if (!ssl_server) 
        {
            debug("Unable to bind to SSL server port %d", wsmand_options_get_server_ssl_port());
            return 1;
        }
        soup_server_add_handler (ssl_server , NULL, &auth_ctx, server_callback, NULL, (env_t *)soap);

        debug("Starting SSL Server on port %d", 
                soup_server_get_port (ssl_server));

        soup_server_run_async (ssl_server);
        g_object_unref (ssl_server);
    }
#endif

#ifdef HAVE_SSL
    if (server == NULL && ssl_server == NULL) {
        fprintf(stderr, "Can't start server.\n");
        debug("Can't start server...");
        return 1;   
    }
#else
    if (server == NULL) {
        fprintf(stderr, "Can't start server.\n");
        debug("Can't start server...");
        return 1;   
    }
#endif

    // WS-Eventing test code
    //start_event_source(soap);
    // End of WS-Eventing test code

    //ws_destroy_context(cntx);
    g_free(listener);
    debug("Waiting for requests...");
    return 0;
}


