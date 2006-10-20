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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include "libsoup/soup.h"
#include "libsoup/soup-session.h"

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-errors.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-client-transport.h"
#include "../client/wsman-client-options.h"



extern ws_auth_request_func_t request_func;


static void
print_header (gpointer name, gpointer value, gpointer user_data)
{
    debug( "[%p]: > %s: %s",
              user_data, (char *) name, (char *) value);
} /* print_header */

static void
http_debug_pre_handler (SoupMessage *message, gpointer user_data)
{
    debug( "[%p]: Receiving response.", message);

    debug( "[%p]: > %d %s",
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
        debug( "[%p]: Response body:\n%.*s\n",
                  message,
                  (int) message->response.length,
                  message->response.body);
    }

    debug( "[%p]: Transfer finished",
              message);
} /* http_debug_post_handler */


static void
http_debug_request_handler (SoupMessage *message, gpointer user_data)
{
    const SoupUri *uri = soup_message_get_uri (message);

    debug(
              "[%p]: > %s %s%s%s HTTP/%s",
              message,
              message->method, uri->path,
              uri->query ? "?" : "",
              uri->query ? uri->query : "",
               "1.1");

    soup_message_foreach_header (message->request_headers,
                                 print_header, message);

    if (message->request.length) {
        debug( "[%p]: Request body:\n%.*s\n",
                  message,
                  (int) message->request.length,
                  message->request.body);
    }

    debug( "[%p]: Request sent.", message);
}

static void
http_debug (SoupMessage *message)
{
    debug( "[%p]: Request queued", message);

    soup_message_add_handler (message, SOUP_HANDLER_POST_REQUEST,
                              http_debug_request_handler, NULL);
    soup_message_add_handler (message, SOUP_HANDLER_PRE_BODY,
                              http_debug_pre_handler, NULL);
    soup_message_add_handler (message, SOUP_HANDLER_PRE_BODY,
                              http_debug_post_handler, NULL);
} /* http_debug */




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
    *username = u_strdup_printf ("%s", user);

    pw = getpass("Password: ");
    *password = u_strdup_printf ("%s", pw);

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
    WsManClient *cl = data;
    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    if (wsc->data.user && wsc->data.pwd) {
        *username = u_strdup (wsc->data.user);
        *password = u_strdup (wsc->data.pwd);
        return;
    }
    reauthenticate(session, msg, auth_type, auth_realm,
           username, password, NULL);
}



void  
wsman_client_handler( WsManClient *cl, 
                      WsXmlDocH rqstDoc, 
                      void* user_data) 
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

    g_signal_connect (session, "authenticate",
                        G_CALLBACK (authenticate), cl);
    g_signal_connect (session, "reauthenticate",
                        G_CALLBACK (reauthenticate), cl);

    msg = soup_message_new_from_uri (SOUP_METHOD_POST,
                        soup_uri_new(wsc->data.endpoint));
    http_debug (msg);
    if (!msg) {
        fprintf (stderr, "Could not parse URI\n");
        return;
    }
    soup_message_add_header(msg->request_headers,
            "User-Agent", wsman_options_get_agent());
    ws_xml_dump_memory_enc(rqstDoc, &buf, &len, "UTF-8");
    soup_message_set_request(msg, SOAP1_2_CONTENT_TYPE,
            SOUP_BUFFER_SYSTEM_OWNED, buf, len);

    // Send the message...        
    soup_session_send_message (session, msg);

    if (msg->status_code != SOUP_STATUS_UNAUTHORIZED &&
                        msg->status_code != SOUP_STATUS_OK) {
        printf ("Connection to server failed: %s (%d)\n",
                msg->reason_phrase, msg->status_code);
    }

    if (msg->response.body) {
        con->response = g_malloc0 (SOUP_MESSAGE (msg)->response.length + 1);
        strncpy (con->response, SOUP_MESSAGE (msg)->response.body,
                        SOUP_MESSAGE (msg)->response.length);
    } 

    g_object_unref (session);
    g_object_unref (msg);

    return;
}

int wsman_client_transport_init(void *arg)
{
    g_type_init ();
    g_thread_init (NULL);
    return 0;
}

void wsman_client_transport_fini()
{
    return;
}

