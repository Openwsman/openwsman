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
 * @author Eugene Yarmosh
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#ifdef PRETTY_OUTPUT
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif

#include <glib.h>
#include "libsoup/soup.h"
#include "libsoup/soup-session.h"

#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"

#include "wsman-client.h"
#include "wsman-debug.h"
#include "wsman-client-options.h"
#include "wsman.h"

void wsman_client_handler( WsManClient *cl, WsXmlDocH rqstDoc, gpointer user_data);




static void
print_header (gpointer name, gpointer value, gpointer user_data)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,
              "[%p]: > %s: %s",
              user_data, (char *) name, (char *) value);
} /* print_header */

static void
http_debug_pre_handler (SoupMessage *message, gpointer user_data)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "[%p]: Receiving response.", message);

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,
              "[%p]: > %d %s",
              message,
              message->status_code,
              message->reason_phrase);

    soup_message_foreach_header (message->response_headers,
                                 print_header, message);
} /* http_debug_pre_handler */

static void
http_debug_post_handler (SoupMessage *message, gpointer user_data)
{
    if (message->response.length) {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,
                  "[%p]: Response body:\n%.*s\n",
                  message,
                  (int) message->response.length,
                  message->response.body);
    }

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,
              "[%p]: Transfer finished",
              message);
} /* http_debug_post_handler */


static void
http_debug_request_handler (SoupMessage *message, gpointer user_data)
{
    const SoupUri *uri = soup_message_get_uri (message);

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,
              "[%p]: > %s %s%s%s HTTP/%s",
              message,
              message->method, uri->path,
              uri->query ? "?" : "",
              uri->query ? uri->query : "",
               "1.1");

    soup_message_foreach_header (message->request_headers,
                                 print_header, message);

    if (message->request.length) {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,
                  "[%p]: Request body:\n%.*s\n",
                  message,
                  (int) message->request.length,
                  message->request.body);
    }

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "[%p]: Request sent.", message);
}

static void
http_debug (SoupMessage *message)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "[%p]: Request queued", message);

    soup_message_add_handler (message, SOUP_HANDLER_POST_REQUEST,
                              http_debug_request_handler, NULL);
    soup_message_add_handler (message, SOUP_HANDLER_PRE_BODY,
                              http_debug_pre_handler, NULL);
    soup_message_add_handler (message, SOUP_HANDLER_PRE_BODY,
                              http_debug_post_handler, NULL);
} /* http_debug */




static void
debug_message_handler (const char *str, WsmanDebugLevel level, gpointer user_data)
{
    if (level <= wsman_options_get_debug_level ()) 
    {
        struct tm *tm;
        time_t now;
        char timestr[128];
        char *log_msg;

        time (&now);
        tm = localtime (&now);
        strftime (timestr, 128, "%b %e %T", tm);

        log_msg = g_strdup_printf ("%s  %s\n",
                                   timestr, str);
       
        fprintf (stderr, log_msg);
        g_free (log_msg);
    }

}

static void
authenticate (SoupSession *session, 
        SoupMessage *msg,
        const char *auth_type, 
        const char *auth_realm,
        char **username, 
        char **password, 
        gpointer data)
{
    // printf("authenticating...\n");
    WsManClient *cl = data;
    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    *username = g_strdup (wsc->data.user);
    *password = g_strdup (wsc->data.pwd);
}


static void
reauthenticate (SoupSession *session, SoupMessage *msg,
        const char *auth_type, const char *auth_realm,
        char **username, char **password, gpointer data)
{
    char *pw;
    char user[21];

    fprintf(stderr,"Authentication failed, please retry\n");
    printf("User name: ");
    fflush(stdout); 
    fgets(user, 20, stdin);

    if (strchr(user, '\n'))
        (*(strchr(user, '\n'))) = '\0';
    *username = g_strdup_printf ("%s", user);

    pw = getpass("Password: ");
    *password = g_strdup_printf ("%s", pw);

}

void  
wsman_client_handler( WsManClient *cl, WsXmlDocH rqstDoc, gpointer user_data) 
{
    SoupSession *session = NULL;
    SoupMessage *msg= NULL;

    char *buf = NULL;
    int len;

    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    WsManConnection *con = wsc->connection;

    if (wsman_options_get_cafile() != NULL) {
        session = soup_session_async_new_with_options (
            SOUP_SESSION_SSL_CA_FILE, wsman_options_get_cafile(),
            NULL);
    } else {
        session = soup_session_async_new ();    
    }

    g_signal_connect (session, "authenticate", G_CALLBACK (authenticate), cl);
    g_signal_connect (session, "reauthenticate", G_CALLBACK (reauthenticate), cl);

    msg = soup_message_new_from_uri (SOUP_METHOD_POST, soup_uri_new(wsc->data.endpoint));
    http_debug (msg);
    if (!msg) {
        fprintf (stderr, "Could not parse URI\n");
        return;
    }
    soup_message_add_header(msg->request_headers, "User-Agent", wsman_options_get_agent());
    ws_xml_dump_memory_enc(rqstDoc, &buf, &len, "UTF-8");
    soup_message_set_request(msg, SOAP1_2_CONTENT_TYPE, SOUP_BUFFER_SYSTEM_OWNED, buf, len);

    // Send the message...        
    soup_session_send_message (session, msg);

    if (msg->status_code != SOUP_STATUS_UNAUTHORIZED && msg->status_code != SOUP_STATUS_OK) {
        printf ("Connection to server failed: %s (%d)\n", msg->reason_phrase, msg->status_code);        
    }

    if (msg->response.body) {    
        con->response = g_malloc0 (SOUP_MESSAGE (msg)->response.length + 1);
        strncpy (con->response, SOUP_MESSAGE (msg)->response.body, SOUP_MESSAGE (msg)->response.length);		
    } 

    g_object_unref (session);
    g_object_unref (msg);

    return;
}

static void
initialize_logging (void)
{    
    wsman_debug_add_handler (debug_message_handler, WSMAN_DEBUG_LEVEL_ALWAYS, NULL);    

} /* initialize_logging */

int main(int argc, char** argv)
{     
    int retVal = 0;   

    g_type_init ();
    g_thread_init (NULL);
    if (!wsman_parse_options(argc, argv))
        return 1;

    initialize_logging ();
    WsContextH cntx = ws_create_runtime(NULL);


    WsManClient *cl;
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Certificate: %s", wsman_options_get_cafile());
    if (wsman_options_get_cafile() != NULL) {
        cl = wsman_connect_with_ssl( cntx, wsman_options_get_server(),
                    wsman_options_get_server_port(),
                    "https",
                    wsman_options_get_username(),
                    wsman_options_get_password(),
                    wsman_options_get_cafile(),
                    NULL,
                    NULL);
    } else {
        cl = wsman_connect( cntx, wsman_options_get_server(),
                    wsman_options_get_server_port(),
                    "http",
                    wsman_options_get_username(),
                    wsman_options_get_password(),
                    NULL);
    }


    if (cl == NULL) {
        fprintf(stderr, "Null Client\n");
    } 


    wsman_client_add_handler(wsman_client_handler, NULL);
    char *resourceUri = wsman_options_get_resource_uri();
    int op = wsman_options_get_action();
    WsXmlDocH doc;
    char *enumContext;
    WsXmlDocH rqstDoc;
    actionOptions options;
    bzero(&options, sizeof(options));
    int optimize_max_elements = 0;
    if (wsman_options_get_dump_request()) {
        options.flags |= FLAG_DUMP_REQUEST;
    }
    if (wsman_options_get_max_envelope_size()) {
        options.max_envelope_size = wsman_options_get_max_envelope_size();
    }
    if (wsman_options_get_operation_timeout()) {
        options.timeout = wsman_options_get_operation_timeout();
    }
    if (wsman_options_get_fragment()) {
        options.fragment = wsman_options_get_fragment();
    }
    if (wsman_options_get_filter()) {
        options.filter = wsman_options_get_filter();
    }
    if (wsman_options_get_dialect()) {
        options.dialect = wsman_options_get_dialect();
    }
    options.cim_ns = wsman_options_get_cim_namespace();


    char *enumeration_mode;

    switch (op) 
    {
    case  ACTION_TEST:
        rqstDoc = ws_xml_read_file(ws_context_get_runtime(cntx), wsman_options_get_test_file(), "UTF-8", 0 );
        doc = ws_send_get_response(cl, rqstDoc, 60000);
        if (doc) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        }    

        break;
    case  ACTION_IDENTIFY: 			
        doc = cl->ft->identify(cl, options);
        if (doc) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        }    
        break;
    case  ACTION_INVOKE: 			
        printf("ResourceUri: %s\n", resourceUri );
        doc = cl->ft->invoke(cl, resourceUri, wsman_options_get_invoke_method(), wsman_options_get_properties(), options);
        if (doc) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        }    
        break;
    case  ACTION_TRANSFER_CREATE: 			
        doc = cl->ft->create(cl, resourceUri, wsman_options_get_properties(), options);
        if (doc) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        }    
        break;
    case  ACTION_TRANSFER_PUT: 			
        doc = cl->ft->put(cl, resourceUri, wsman_options_get_properties(), options);        		        		
        if (doc) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        }    
        break;
    case  ACTION_TRANSFER_GET: 			
        doc = cl->ft->get(cl, resourceUri, options);
        if (doc) {
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
        }    
        break;
    case ACTION_ENUMERATION:

        enumeration_mode = wsman_options_get_enum_mode();
        if (enumeration_mode) {
            if (strcmp(enumeration_mode, "epr") == 0 ) 
                options.flags |= FLAG_ENUMERATION_ENUM_EPR;
            else
                options.flags |= FLAG_ENUMERATION_ENUM_OBJ_AND_EPR;
        }
        if (wsman_options_get_optimize_enum()) {
            options.flags |= FLAG_ENUMERATION_OPTIMIZATION;
            optimize_max_elements = wsman_options_get_max_elements();
        }
        if (wsman_options_get_estimate_enum())
            options.flags |= FLAG_ENUMERATION_COUNT_ESTIMATION;
        
        WsXmlDocH enum_response = cl->ft->wsenum_enumerate(cl, resourceUri, optimize_max_elements, options);
        enumContext = wsenum_get_enum_context(enum_response); 
        WsXmlNodeH cntxNode;
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "enumContext: %s", enumContext );
        if (enumContext) {
            int counter = 1;
            while( (doc = cl->ft->wsenum_pull(cl, resourceUri, enumContext, wsman_options_get_max_elements(), options) )) {
                WsXmlNodeH node = ws_xml_get_child(ws_xml_get_soap_body(doc), 0, NULL, NULL);
                ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
                if ( strcmp(ws_xml_get_node_local_name(node), WSENUM_PULL_RESP) != 0 ) {                		                		
                    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "no pull response" );
                    break;
                }
                if ( ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE) ) {                		
                    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "End of sequence");
                    break;
                }
                if ( (cntxNode = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT)) ) {
                    soap_free(enumContext);
                    enumContext = soap_clone_string(ws_xml_get_node_text(cntxNode));
                }                				
                if ( enumContext == NULL || enumContext[0] == 0 ) {
                    wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "No enumeration context");
                    break;
                }
                /*
                if (counter == 3 ) {
                    enumContext = "xxxx";
                }
                */
                counter++;
            }
        } else {
            if (enum_response)
                ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(enum_response));
        }    
        break;
    default:
        fprintf(stderr, "Action not supported\n");    		
        retVal = 1;
    }    


    cl->ft->release(cl);   
        
    if (doc)
        ws_xml_destroy_doc(doc);
    //soap_destroy_fw(cntx);
    //soap_free(cntx);

    return retVal;

}
