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
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "u/libu.h"
#include "wsman-client-transport.h"
#include "wsman-soap.h"
#include "wsman-client.h"

static char *auth_methods[] = {
	"noauth",
	"basic",
	"digest",
	"pass",
	"ntlm",
	"gss",
	NULL,
};


extern void wsmc_handler(WsManClient * cl, WsXmlDocH rqstDoc,
				 void *user_data);

#ifdef BENCHMARK
static long long transfer_time = 0;
#endif

int wsman_send_request(WsManClient * cl, WsXmlDocH request)
{
#ifdef BENCHMARK
	struct timeval tv0, tv1;
	long long t0, t1;
#endif

	if (wsmc_lock(cl) != 0 ) {
		error("Client busy");
		return 1;
	}
	wsmc_reinit_conn(cl);

#ifdef BENCHMARK
	gettimeofday(&tv0, NULL);
#endif

	wsmc_handler(cl, request, NULL);

#ifdef BENCHMARK
	gettimeofday(&tv1, NULL);
	t0 = tv0.tv_sec * 10000000 + tv0.tv_usec;
	t1 = tv1.tv_sec * 10000000 + tv1.tv_usec;
	transfer_time += t1 - t0;
#endif
	wsmc_unlock(cl);
	return 0;
}

#ifdef BENCHMARK
long long get_transfer_time()
{
	long long l = transfer_time;
	transfer_time = 0;
	return l;
}
#endif


char *wsmc_transport_get_auth_name(wsman_auth_type_t auth)
{
	switch (auth) {
	case WS_NO_AUTH:
		return _WS_NO_AUTH;
	case WS_BASIC_AUTH:
		return _WS_BASIC_AUTH;
	case WS_DIGEST_AUTH:
		return _WS_DIGEST_AUTH;
	case WS_PASS_AUTH:
		return _WS_PASS_AUTH;
	case WS_NTLM_AUTH:
		return _WS_NTLM_AUTH;
	case WS_GSSNEGOTIATE_AUTH:
		return _WS_GSSNEGOTIATE_AUTH;
	default:;
	}
	return "Unknown";
}

void wsmc_transport_set_auth_request_func(WsManClient * cl,
				  wsman_auth_request_func_t f)
{
	cl->authentication.auth_request_func = f;
}


void wsman_transport_set_agent(WsManClient * cl, const char *arg)
{
	cl->user_agent = arg ? u_strdup( arg ) : NULL;
}

char *wsman_transport_get_agent(WsManClient * cl)
{
	if (cl->user_agent)
		return cl->user_agent;
	else
		return DEFAULT_USER_AGENT;
}


void wsman_transport_set_proxy(WsManClient * cl, const char *arg)
{
	cl->proxy_data.proxy = arg ? u_strdup( arg ) : NULL;
}

char *wsman_transport_get_proxy(WsManClient *cl)
{
	return cl->proxy_data.proxy ? u_strdup( cl->proxy_data.proxy ) : NULL;
}

void wsman_transport_set_userName(WsManClient * cl, char *arg)
{
	if(arg) {
		cl->data.user = u_strdup(arg);
	}
}

void wsman_transport_set_password(WsManClient * cl, char *arg)
{
	if(arg) {
		cl->data.pwd = u_strdup(arg);
	}
}

void wsman_transport_set_proxyauth(WsManClient * cl, const char *arg)
{
	cl->proxy_data.proxy_auth = arg ? u_strdup( arg ) : NULL;
}

char *wsman_transport_get_proxyauth(WsManClient *cl)
{
	return cl->proxy_data.proxy_auth ? u_strdup( cl->proxy_data.proxy_auth ) : NULL;
}


unsigned long wsman_transport_get_timeout(WsManClient * cl)
{
	return cl->transport_timeout;
}

void wsman_transport_set_timeout(WsManClient * cl, unsigned long arg)
{
	cl->transport_timeout = arg;
}


char *wsman_transport_get_auth_method(WsManClient * cl)
{
	return cl->authentication.method;
}

void wsman_transport_set_auth_method(WsManClient * cl, const char *arg)
{
	cl->authentication.method = arg ? u_strdup( arg ) : NULL;
}

int wsman_is_auth_method(WsManClient * cl, int method)
{
	if (cl->authentication.method == NULL) {
		return 1;
	}
	if (method >= WS_MAX_AUTH) {
		return 0;
	}
	return (!strncasecmp
		(cl->authentication.method, auth_methods[method],
		 strlen(cl->authentication.method)));
}

wsman_auth_type_t wsmc_transport_get_auth_value(WsManClient * cl)
{
	char *m = cl->authentication.method;
	wsman_auth_type_t i;

	if (m == NULL) {
		return WS_NO_AUTH;
	}
	for (i = 0; auth_methods[i] != NULL; i++) {
		if (!strcasecmp(m, auth_methods[i])) {
			return i;
		}
	}
	return WS_MAX_AUTH;
}


void wsman_transport_set_verify_peer(WsManClient * cl, unsigned int arg)
{
	cl->authentication.verify_peer = arg;
}

unsigned int wsman_transport_get_verify_peer(WsManClient *cl)
{
	return cl->authentication.verify_peer;
}


void wsman_transport_set_verify_host(WsManClient * cl, unsigned int arg)
{
	cl->authentication.verify_host = arg;
}

unsigned int wsman_transport_get_verify_host(WsManClient *cl)
{
	return cl->authentication.verify_host;
}

void wsman_transport_set_crlcheck(WsManClient * cl, unsigned int arg)
{
	cl->authentication.crl_check = arg;
}

unsigned int wsman_transport_get_crlcheck(WsManClient * cl)
{
        return cl->authentication.crl_check;
}

#ifndef _WIN32
void wsman_transport_set_crlfile(WsManClient * cl, const char *arg)
{
        u_free(cl->authentication.crl_file);
        cl->authentication.crl_file = arg ? u_strdup( arg ) : NULL;
}

char *wsman_transport_get_crlfile(WsManClient *cl)
{
        return cl->authentication.crl_file; 
}
#endif



void wsman_transport_set_cainfo(WsManClient * cl, const char *arg)
{
	cl->authentication.cainfo = arg ? u_strdup( arg ) : NULL;
}

char *wsman_transport_get_cainfo(WsManClient *cl)
{
	return cl->authentication.cainfo ? u_strdup( cl->authentication.cainfo ) : NULL;
}

static int hexadecimal2raw(const char *str, unsigned char *dest, int len) {
        int i = 0;
        unsigned char v1 =0 , v2 = 0;
        while(*str != 0 && *(str+1) != 0 && i < len) {
                if(*str <= '9' && *str >= '0')
                        v1 = *str - '0';
                else if(*str <= 'f' && *str >= 'a')
                        v1 = *str - 'a' + 10;
                else if(*str <= 'F' && *str >= 'A')
                        v1 = *str - 'A' + 10;
                v1 <<= 4;
                str++;
                if(*str <= '9' && *str >= '0')
                        v2 = *str - '0';
                else if(*str <= 'f' && *str >= 'a')
                        v2 = *str - 'a' + 10;
                else if(*str <= 'F' && *str >= 'A')
                        v2 = *str - 'A' + 10;
                str++;
                dest[i] = v1 + v2;
                i++;
        }
        return i;
}

static char * raw2hexadecimal(unsigned char *dest, int len)
{
        int i = 0;
        char v;
        char * str = calloc(1, len*2+1);
        char * p = str;
        if(str == NULL) return str;
        while(i < len) {
                v = (dest[i] & 0xf0) >> 4;
                if(v <= 9)
                        *str = '0' + v;
                else
                        *str = 'a' + v - 10;
                str++;
                v = dest[i] & 0x0f;
                if(v <= 9)
                        *str = '0' + v;
                else
                        *str = 'a' + v - 10;
                str++;
                i++;
        }
        return p;
}


void wsman_transport_set_certhumbprint(WsManClient *cl, const char *arg)
{
	if(arg == NULL)
		return;
	hexadecimal2raw(arg, cl->authentication.certificatethumbprint, 20);
}

char *wsman_transport_get_certhumbprint(WsManClient *cl)
{

	return raw2hexadecimal(cl->authentication.certificatethumbprint, 20);
}

void wsman_transport_set_capath(WsManClient *cl, const char *arg)
{
	cl->authentication.capath = arg ? u_strdup( arg ) : NULL;
}

char *wsman_transport_get_capath(WsManClient *cl)
{
	return cl->authentication.capath ? u_strdup( cl->authentication.capath ) : NULL;
}


void wsman_transport_set_caoid(WsManClient *cl, const char *arg)
{
	cl->authentication.caoid = arg ? u_strdup( arg ) : NULL;
}

char *wsman_transport_get_caoid(WsManClient *cl)
{
	return cl->authentication.caoid ? u_strdup( cl->authentication.caoid ) : NULL;
}


#ifdef _WIN32
void wsman_transport_set_calocal(WsManClient *cl, BOOL local)
{
	cl->authentication.calocal = local;
}

BOOL wsman_transport_get_calocal(WsManClient *cl)
{
	return cl->authentication.calocal;
}
#endif

void wsman_transport_set_proxy_username(WsManClient *cl, char *proxy_username )
{
        cl->proxy_data.proxy_username = proxy_username;
}
void wsman_transport_set_proxy_password(WsManClient *cl, char *proxy_password )
{
        cl->proxy_data.proxy_password = proxy_password;
}



void wsman_transport_set_cert(WsManClient * cl, const char *arg)
{
	cl->authentication.sslcert = arg ? u_strdup( arg ) : NULL;
}

char *wsman_transport_get_cert(WsManClient *cl)
{
	return cl->authentication.sslcert ? u_strdup( cl->authentication.sslcert ) : NULL;
}


void wsman_transport_set_key(WsManClient *cl, const char *key)
{
	cl->authentication.sslkey = key ? u_strdup( key ) : NULL;
}

char *wsman_transport_get_key(WsManClient *cl)
{
	return cl->authentication.sslkey ? u_strdup( cl->authentication.sslkey ) : NULL;
}


char *wsman_transport_get_last_error_string(WS_LASTERR_Code err)
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
