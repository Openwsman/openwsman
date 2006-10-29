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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wsman-client-transport.h"
#include "u/libu.h"

char *auth_methods[] = {
     "noauth",
     "basic",
     "digest",
     "ntlm",
     NULL,
};

static char *authentication_method = NULL;
static char *proxy = NULL;
static char *proxy_auth = NULL;
static char *user_agent = PACKAGE_STRING;
static int noverifypeer = 0;
static char *cafile;

static long long transfer_time = 0;

long long
get_transfer_time()
{
    long long l = transfer_time;
    transfer_time = 0;
    return l;
}




static WsManConnection* 
init_client_connection(WsManClientData *cld)
{
   WsManConnection *conn =(WsManConnection*)u_zalloc(sizeof(WsManConnection));
   u_buf_create(&conn->response);
   u_buf_create(&conn->request);

   return conn;
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

  u_free(cl);
  return rc;
}

static void
request_usr_pwd(ws_auth_type_t auth,
                char **username,
                char **password);

ws_auth_request_func_t request_func = &request_usr_pwd;


static void
request_usr_pwd(ws_auth_type_t auth,
                char **username,
                char **password)
{
    char *pw;
    char user[21];

    fprintf(stdout,"Authentication failed, please retry\n");
    fprintf(stdout, "%s authentication is used\n",
                ws_client_transport_get_auth_name(auth));
    printf("User name: ");
    fflush(stdout); 
    fgets(user, 20, stdin);

    if (strchr(user, '\n'))
        (*(strchr(user, '\n'))) = '\0';
    *username = u_strdup_printf ("%s", user);

    pw = getpass("Password: ");
    *password = u_strdup_printf ("%s", pw);
}

char *ws_client_transport_get_auth_name(ws_auth_type_t auth)
{
    switch (auth) {
        case WS_NO_AUTH :    return "No Auth";
        case WS_BASIC_AUTH:  return "Basic";
        case WS_DIGEST_AUTH: return "Digest";
        case WS_NTLM_AUTH:   return "NTLM";
        default: ;
    }
    return "Unknown";
}

void ws_client_transport_set_auth_request_func(ws_auth_request_func_t f)
{
    request_func = f;
}

int wsman_is_auth_method(int method)
{
    if (authentication_method == NULL) {
        return 1;
    }
    if (method >= WS_MAX_AUTH) {
        return 0;
    }
    return (!strncasecmp(authentication_method, auth_methods[method],
            strlen(authentication_method)));
}

void
wsman_client(WsManClient *cl, WsXmlDocH rqstDoc)
{
    struct timeval tv0, tv1;
    long long t0, t1;

    gettimeofday(&tv0, NULL);
    
    wsman_client_handler(cl, rqstDoc, NULL);
    
    gettimeofday(&tv1, NULL);
    t0 = tv0.tv_sec * 10000000 + tv0.tv_usec;
    t1 = tv1.tv_sec * 10000000 + tv1.tv_usec;
    transfer_time += t1 -t0;

    return;
}

char *wsman_transport_get_proxy()
{
    return proxy;
}

char *wsman_transport_get_proxyauth()
{
    return proxy_auth;
}

char * wsman_transport_get_agent ()
{
    return user_agent;
}

char * wsman_transport_get_auth_method ()
{
    return authentication_method;
}

int wsman_transport_get_no_verify_peer ()
{
    return noverifypeer;
}

char *wsman_transport_get_cafile()
{
    return cafile;
}


void wsman_transport_set_proxy(char *arg)
{
    proxy = arg;
}

void wsman_transport_set_proxyauth(char *arg)
{
    proxy_auth = arg;
}

void wsman_transport_set_agent (char *arg)
{
    user_agent = arg;
}

void wsman_transport_set_auth_method (char *arg)
{
    authentication_method = arg;
}

void wsman_transport_set_no_verify_peer (int arg)
{
    noverifypeer = arg;
}

void wsman_transport_set_cafile(char *arg)
{
    cafile = arg;
}





