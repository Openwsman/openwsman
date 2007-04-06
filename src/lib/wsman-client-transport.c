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

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#else
#ifndef PACKAGE_STRING
#define PACKAGE_STRING "openwsman client"
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "wsman-client-transport.h"
#include "wsman-soap.h"
#include "wsman-client.h"
#include "u/libu.h"

char *auth_methods[] = {
  "noauth",
  "basic",
  "digest",
  "pass",
  "ntlm",
  "gss",
  NULL,
};

static char *authentication_method = NULL;
static char *proxy = NULL;
static char *proxy_auth = NULL;
static char *user_agent = PACKAGE_STRING;
static int noverifypeer = 0;
static unsigned long  transport_timeout = 0;
static char *cafile;

extern void wsman_client_handler( WsManClient *cl, WsXmlDocH rqstDoc, void* user_data);


static long long transfer_time = 0;

int
wsman_send_request(WsManClient *cl, WsXmlDocH request)
{
  struct timeval tv0, tv1;
  long long t0, t1;

  if (wsman_client_lock(cl)) {
        error("Client busy");
        return 1;
  }
  reinit_client_connection(cl);

  gettimeofday(&tv0, NULL);

  wsman_client_handler(cl, request, NULL);

  gettimeofday(&tv1, NULL);
  t0 = tv0.tv_sec * 10000000 + tv0.tv_usec;
  t1 = tv1.tv_sec * 10000000 + tv1.tv_usec;
  transfer_time += t1 -t0;
  wsman_client_unlock(cl);
  return 0;
}

long long
get_transfer_time()
{
  long long l = transfer_time;
  transfer_time = 0;
  return l;
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
  char *p;

  fprintf(stdout,"Authentication failed, please retry\n");
  fprintf(stdout, "%s authentication is used\n",
          ws_client_transport_get_auth_name(auth));
  printf("User name: ");
  fflush(stdout); 
  if ( (p = fgets(user, 20, stdin) ) != NULL ) 
  {

    if (strchr(user, '\n'))
      (*(strchr(user, '\n'))) = '\0';
    *username = u_strdup_printf ("%s", user);
  } else {
    *username = NULL;
  }

  pw = (char *)getpass("Password: ");
  *password = u_strdup_printf ("%s", pw);
}

char *ws_client_transport_get_auth_name(ws_auth_type_t auth)
{
  switch (auth) {
  case WS_NO_AUTH :    return "No Auth";
  case WS_BASIC_AUTH:  return "Basic";
  case WS_DIGEST_AUTH: return "Digest";
  case WS_NTLM_AUTH:   return "NTLM";
  case WS_GSSNEGOTIATE_AUTH : return "GSS-Negotiate";
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

int ws_client_transport_get_auth_value()
{
    char *m = authentication_method;
    ws_auth_type_t i;

    if (m == NULL) {
        return 0;
    }
    for (i = 0; auth_methods[i] != NULL; i++) {
        if (!strcasecmp(m, auth_methods[i])) {
            return i;
        }
    }
    return 0;
}

char *wsman_transport_get_proxy()
{
  return proxy;
}

char *wsman_transport_get_proxyauth()
{
  return proxy_auth;
}

unsigned long wsman_transport_get_timeout()
{
  return transport_timeout;
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

void wsman_transport_set_timeout(unsigned long arg)
{
  transport_timeout = arg;
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


char *
wsman_transport_get_last_error_string(WS_LASTERR_Code err)
{
    switch (err) {
        case WS_LASTERR_OK:
            return "Everithing OK";
        case WS_LASTERR_FAILED_INIT:
            return "Trnasport initailization failed";
        case WS_LASTERR_UNSUPPORTED_PROTOCOL:
            return "Unsupported protocol";
        case WS_LASTERR_URL_MALFORMAT:
            return "URL malformat";
        case WS_LASTERR_COULDNT_RESOLVE_PROXY:
            return "Could not resolve proxy";
        case WS_LASTERR_COULDNT_RESOLVE_HOST:
            return "Could not resolve host";
        case WS_LASTERR_COULDNT_CONNECT:
            return "Could not connect";
        case WS_LASTERR_HTTP_RETURNED_ERROR:
            return "HTTP returned error";
        case WS_LASTERR_WRITE_ERROR:
            return "Write error";
        case WS_LASTERR_READ_ERROR:
            return "Read error";
        case WS_LASTERR_OUT_OF_MEMORY:
            return "Could not alloc memory";
        case WS_LASTERR_OPERATION_TIMEOUTED:
            return "Operation timeout reached";
        case WS_LASTERR_HTTP_POST_ERROR:
            return "HTTP POST error";
        case WS_LASTERR_BAD_DOWNLOAD_RESUME:
            return "Couldn't resume download";
        case WS_LASTERR_TOO_MANY_REDIRECTS:
            return "Catch endless re-direct loop";
        case WS_LASTERR_SSL_CONNECT_ERROR:
            return "SSL connection error";
        case WS_LASTERR_SSL_PEER_CERTIFICATE:
            return "Peer's certificate wasn't OK";
        case WS_LASTERR_SSL_ENGINE_NOTFOUND:
            return "SSL crypto engine not found";
        case WS_LASTERR_SSL_ENGINE_SETFAILED:
            return "Can't set SSL crypto engine default";
        case WS_LASTERR_SSL_CERTPROBLEM:
            return "Problem with the local certificate";
        case WS_LASTERR_SSL_CACERT:
            return "Problem with the CA certificate";
        case WS_LASTERR_SSL_ENGINE_INITFAILED:
            return " failed to initialise SSL engine";
        case WS_LASTERR_SEND_ERROR:
            return "Failed sending network data";
        case WS_LASTERR_RECV_ERROR:
            return "Failure in receiving network data";
        case WS_LASTERR_BAD_CONTENT_ENCODING:
            return "Unrecognized transfer encoding";
        case WS_LASTERR_LOGIN_DENIED:
            return "User, password or similar was not accepted";
        default:
            return "Unrecognized error";
    }
}







