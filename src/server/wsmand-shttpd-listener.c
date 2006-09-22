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
 * @author Vadim Revyakin
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


#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"

#include "wsman-xml.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"

#define OPENWSMAN
#include "shttpd.h"

#include "wsman-plugins.h"
#include "wsmand-listener.h"
#include "wsmand-daemon.h"
#include "wsmand-auth.h"
#include "wsman-server.h"
#include "wsman-plugins.h"



#if 0
static void
print_header (gpointer name, gpointer value, gpointer data)
{
    debug( "%s: %s", (char *)name, (char *)value);
}


static gboolean
server_auth_callback ( SoupServerAuthContext *auth_ctx, SoupServerAuth *auth,
        SoupMessage  *msg, gpointer data) 
{
    gboolean authorized = FALSE;
    char *filename = NULL;
    soup_message_foreach_header (msg->request_headers, print_header, NULL);
    soup_message_add_header (msg->response_headers, "Server", PACKAGE"/"VERSION );
    soup_message_add_header (msg->response_headers, "Content-Type", "application/soap+xml;charset=UTF-8"); 

#if 0 
    WsXmlDocH inDoc = wsman_build_inbound_envelope( (env_t *)data, msg);
    if (wsman_is_identify_request(inDoc)) {
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
            if (filename == NULL) {
                break;
            }
            authorized = ws_authorize_basic(filename,
                (char *)soup_server_auth_get_user (auth), auth->basic.passwd);
            break;
        case SOUP_AUTH_TYPE_DIGEST: {
                WSmanAuthDigest digest;
                digest.request_method = auth->digest.request_method;
                digest.digest_uri = auth->digest.digest_uri;            
                digest.username = auth->digest.user;            
                digest.realm = auth->digest.realm;            
                digest.integrity = auth->digest.integrity;            
                digest.nonce_count = auth->digest.nonce_count;            
                digest.nonce = auth->digest.nonce;            
                digest.cnonce = auth->digest.cnonce;            
                digest.digest_response = auth->digest.digest_response;
                if (filename == NULL) {
                        break;
                }            
                filename = wsmand_options_get_digest_password_file();
                authorized = ws_authorize_digest(filename, &digest);
            }
            break;
        }
    } 

    if (!authorized) {
        soup_message_add_header (msg->response_headers, "Content-Length", "0" );
        soup_message_add_header (msg->response_headers, "Connection", "close" );
    }  
    return authorized;
}

#endif



static int
digest_auth_callback(char *realm, char *method, struct digest *dig)
{
        WSmanAuthDigest wsdig;
        char *filename = wsmand_options_get_digest_password_file();
        
        if (filename == NULL) {
                debug( "Could not get digest password file name");
                return 0;
        }       
        wsdig.request_method = method;
        wsdig.username       = dig->user;
        wsdig.realm          = realm;
        wsdig.digest_uri     = dig->uri;
        wsdig.nonce          = dig->nonce;
        wsdig.cnonce         = dig->cnonce;
        wsdig.qop            = dig->qop;
        strncpy(wsdig.nonce_count, dig->nc, sizeof (wsdig.nonce_count));
        wsdig.digest_response = dig->resp;

        return ws_authorize_digest(filename, &wsdig);
}
        
        

static int
basic_auth_callback(char *username, char *password)
{
        char *filename = wsmand_options_get_basic_password_file();
        
        if (filename == NULL) {
                debug(
                     "Could not get password file name");
                return 0;
        }
        debug( "username : [%s] ; passwd [%s]",
                        username, password);
        return ws_authorize_basic(filename, username, password);
}



static char *
shttp_reason_phrase(int status) {
        return "Error";
}

typedef struct {
      char *response;
      int length;
      int ind;
}   ShttpMessage;



static int server_callback (struct shttpd_arg_t *arg)
{
    const char *path, *method;
    char *default_path;
    const char *content_type;
    const char *encoding;
    
    
    int status = WSMAN_STATUS_OK;

    ShttpMessage *shttp_msg = (ShttpMessage *)arg->state;
    WsmanMessage *wsman_msg;
    int n = 0;
    int k;
    
    if (shttp_msg != NULL) {
        // WE already have the response. Just continue to send it to server
        goto CONTINUE;
    }
    
    // Here we must handle the initial request
    
    wsman_msg = wsman_soap_message_new();

    // Check HTTP headers
    path = shttpd_get_uri(arg);
    method = shttpd_get_env(arg, "REQUEST_METHOD");
    debug("%s %s HTTP/1.%d", method, path,
            shttpd_get_http_version(arg));

//    soup_message_foreach_header (msg->request_headers, print_header, NULL);
//    if (msg->request.length) {
//        debug("Request: %.*s", msg->request.length, msg->request.body);
//    }

    // Check Method
    if (strncmp(method, "POST", 4)) {
        debug( "Insupported method %s", method);
        status = WSMAN_STATUS_NOT_IMPLEMENTED;
        goto DONE;
    }   

    default_path = wsmand_options_get_service_path();
    if (path) {
        if (strcmp(path, "/wsman") != 0 ) {
            status = WSMAN_STATUS_BAD_REQUEST;
            goto DONE;
        }
    } else {
        path = g_strdup ("");
    }    

    content_type = shttpd_get_header(arg, "Content-Type");
    if (content_type && strncmp(content_type, SOAP_CONTENT_TYPE, strlen(SOAP_CONTENT_TYPE)) != 0 ) {
        status = WSMAN_STATUS_BAD_REQUEST;
        goto DONE;
    }
    encoding = strchr(content_type, '=') + 1;
     debug("Encoding: %s", encoding);


    env_t* fw = (env_t*)arg->user_data;	
    wsman_msg->status.fault_code = WSMAN_RC_OK;

    wsman_msg->request.length = shttpd_get_post_query_len(arg);
    wsman_msg->request.body   = (char *)malloc(wsman_msg->request.length);
    if (wsman_msg->request.body == NULL) {
        status = WSMAN_STATUS_INTERNAL_SERVER_ERROR;
        goto DONE;
    }
    (void) shttpd_get_post_query(arg, wsman_msg->request.body,
                    wsman_msg->request.length);    
    
    /*
    wsman_msg->auth_data.username = soup_server_auth_get_user(context->auth);
    wsman_msg->auth_data.password = context->auth->basic.passwd;
    */
{int i; char *p = wsman_msg->request.body;
printf("   ****  wsman_msg->request.body ******\n");
while(p - wsman_msg->request.body < wsman_msg->request.length) {
        printf("%s\n", p);
        p = strchr(p, '\0');
        if (p == NULL) break;
        p++;
}
printf("   ****  wsman_msg->request.body end ******\n");
}
    // Call dispatcher
    dispatch_inbound_call(fw, wsman_msg);

    if (wsman_msg->request.body) {
        free(wsman_msg->request.body);
        wsman_msg->request.body = NULL;
    }
    wsman_msg->request.length = 0;
    
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
            shttp_msg = (ShttpMessage *)malloc(sizeof (ShttpMessage));
            if (shttp_msg) {
                shttp_msg->length = len;
                shttp_msg->response = strndup(buf, len);
                shttp_msg->ind = 0;
            } else {
                //  XXX handle error
            }  
            free(buf);
        }

        status = WSMAN_STATUS_INTERNAL_SERVER_ERROR; // ?????
        goto DONE;
    }
    
    shttp_msg = (ShttpMessage *)malloc(sizeof (ShttpMessage));
    if (shttp_msg) {
        shttp_msg->length = wsman_msg->response.length;
        shttp_msg->response = (char *)wsman_msg->response.body;
        shttp_msg->ind = 0;
        status =wsman_msg->http_code;
    } else {
        status = WSMAN_STATUS_INTERNAL_SERVER_ERROR;
    }  

    debug("Response: %s", (char *)wsman_msg->response.body);



    if (wsman_msg->in_doc != NULL) {
        ws_xml_destroy_doc(wsman_msg->in_doc);
    }
DONE:

    if (wsman_msg) free(wsman_msg);
    debug( 
            "Response (status) %d", status, shttp_reason_phrase(status));
            
    
    // Here we begin to create response
    // we consider output buffer is large enough to hold all headers
    switch (status) {
        case WSMAN_STATUS_OK:
                n += snprintf(arg->buf + n, arg->buflen -n,
	               "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n",
                        SOAP1_2_CONTENT_TYPE);
                break;
       default:
	       n += snprintf(arg->buf + n, arg->buflen -n, "HTTP/1.1 %d %s\r\n",
                        status, shttp_reason_phrase(status));
        break;
    }
//    n += snprintf(arg->buf + n, arg->buflen -n, "Server

    if (!shttp_msg || shttp_msg->length == 0) {
        n += snprintf(arg->buf + n, arg->buflen -n, "\r\n\r\n");
        arg->last = 1;
        if (shttp_msg) free(shttp_msg);
        return n;
    }
    
    n += snprintf(arg->buf + n, arg->buflen -n, "\r\n");
    
CONTINUE:
    k = arg->buflen - n;
    if (k <= shttp_msg->length - shttp_msg->ind) {
        // not enogh room for all message. transfer only part
        memcpy(arg->buf + n, shttp_msg->response + shttp_msg->ind, k);
        shttp_msg->ind += k;
        arg->state = shttp_msg;
        return n + k;
    }
    // Enough room for all message
    memcpy(arg->buf + n, shttp_msg->response + shttp_msg->ind,
                                shttp_msg->length - shttp_msg->ind);
    n += shttp_msg->length - shttp_msg->ind;
    if (n + 4 > arg->buflen) {
        // not enough room empty lines at the end of message
        arg->state = shttp_msg;
        shttp_msg->ind = shttp_msg->length;
        return n;
    }
    
    // here we can complete
    n += snprintf(arg->buf + n, arg->buflen -n, "\r\n\r\n");
    free(shttp_msg->response);
    free(shttp_msg);
    arg->last =1;
    arg->state = NULL;
    return n;
}


static void
listener_shutdown_handler(gpointer p)
{
        int *a = (int *)p;
        debug(
                "listener_shutdown_handler started");
        *a = 0;
}


WsManListenerH*
wsmand_start_server() 
{
    WsManListenerH *listener = wsman_dispatch_list_new();
    WsContextH cntx = wsman_init_plugins(listener);
    
    if (cntx == NULL) {
        return NULL;
    }
    SoapH soap = ws_context_get_runtime(cntx);    
#if 0	
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
#endif

    struct shttpd_ctx   *ctx;
    int sock;
    int port;
    
    static int continue_working = 1;
    
    
    port = wsmand_options_get_server_port();
    if (port == 0) {
        port = 9001;
    }
    ctx = shttpd_init(NULL,
        // "ssl_certificate", "/home/shttpd-1.35/shttpd.pem",
          "debug", "1",
          NULL);
    if (ctx == NULL) {
         return NULL;
    }
    
    shttpd_register_url(ctx, "/wsman", server_callback, (void *) soap);
    debug( "     Working on port %d", port);

    if (wsmand_options_get_digest()) {
        debug( "Using Digest Authorization - not implemented now");
    } else {
        shttpd_register_bauth_callback(ctx, basic_auth_callback);
        debug( "Using Basic Authorization");
    }   
    sock = shttpd_open_port(port);
    shttpd_listen(ctx, sock);
    
    wsmand_shutdown_add_handler(listener_shutdown_handler, &continue_working);
    
    while (continue_working) {
           shttpd_poll(ctx, 1000);
    }
    debug( "shttpd_poll loop canceled");
    g_free(listener);
    return listener;
}


