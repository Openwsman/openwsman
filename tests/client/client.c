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
wsman_client_handler(
			WsManClient *cl, 
			WsXmlDocH rqstDoc,
			gpointer user_data) 
{
	
    SoupSession *session = NULL;
    SoupMessage *msg= NULL;
   
    char *buf = NULL;
    int len;
    
    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    WsManConnection *con = wsc->connection;
		
    session = soup_session_async_new ();    
    g_signal_connect (session, "authenticate",
            G_CALLBACK (authenticate), cl);
    g_signal_connect (session, "reauthenticate",
            G_CALLBACK (reauthenticate), cl);
                    
    SoupUri *ep = soup_uri_new(wsc->data.endpoint);
    msg = soup_message_new_from_uri (SOUP_METHOD_POST, ep);
    if (!msg) 
    {
        fprintf (stderr, "Could not parse URI\n");
        return;
    }

	
    // FIXME: define in header file and use real version
    soup_message_add_header(msg->request_headers, "User-Agent", "OpenWSMAN/0.01");
	// ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc), 1);
    
    ws_xml_dump_memory_enc(rqstDoc, &buf, &len, "UTF-8");
        
    soup_message_set_request(msg, 
            SOAP1_2_CONTENT_TYPE,
            SOUP_BUFFER_SYSTEM_OWNED,
            buf,
            len);
                   	                 
    // Send the message...        
    soup_session_send_message (session, msg);
  
    if (msg->status_code != SOUP_STATUS_UNAUTHORIZED &&
            msg->status_code != SOUP_STATUS_OK) 
    {
        printf ("Connection to server failed: %s (%d)\n", 
        			msg->reason_phrase, 
        			msg->status_code);        
    }
    
    if (msg->response.body) 
    {    
    		con->response = g_malloc0 (SOUP_MESSAGE (msg)->response.length + 1);
		strncpy (con->response, SOUP_MESSAGE (msg)->response.body, SOUP_MESSAGE (msg)->response.length);		
    } 
	
    g_object_unref (session);
    g_object_unref (msg);
    g_free(ep);
    
    return;
}

