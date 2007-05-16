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



extern void wsmc_handler(WsManClient * cl, WsXmlDocH rqstDoc,
				 void *user_data);

#ifdef DEBUG_VERBOSE
static long long transfer_time = 0;
#endif

int wsman_send_request(WsManClient * cl, WsXmlDocH request)
{
#ifdef DEBUG_VERBOSE
	struct timeval tv0, tv1;
	long long t0, t1;
#endif

	if (wsmc_lock(cl)) {
		error("Client busy");
		return 1;
	}
	wsmc_reinit_conn(cl);

#ifdef DEBUG_VERBOSE
	gettimeofday(&tv0, NULL);
#endif

	wsmc_handler(cl, request, NULL);

#ifdef DEBUG_VERBOSE
	gettimeofday(&tv1, NULL);
	t0 = tv0.tv_sec * 10000000 + tv0.tv_usec;
	t1 = tv1.tv_sec * 10000000 + tv1.tv_usec;
	transfer_time += t1 - t0;
#endif
	wsmc_unlock(cl);
	return 0;
}

#ifdef DEBUG_VERBOSE
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
		return "No Auth";
	case WS_BASIC_AUTH:
		return "Basic";
	case WS_DIGEST_AUTH:
		return "Digest";
	case WS_NTLM_AUTH:
		return "NTLM";
	case WS_GSSNEGOTIATE_AUTH:
		return "GSS-Negotiate";
	default:;
	}
	return "Unknown";
}

void wsmc_transport_set_auth_request_func(WsManClient * cl,
						  wsman_auth_request_func_t
						  f)
{
	cl->authentication.auth_request_func = f;
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

int wsmc_transport_get_auth_value(WsManClient * cl)
{
	char *m = cl->authentication.method;
	wsman_auth_type_t i;

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

char *wsman_transport_get_proxy(WsManClient * cl)
{
	return cl->proxy_data.proxy;
}

char *wsman_transport_get_proxyauth(WsManClient * cl)
{
	return cl->proxy_data.proxy_auth;
}

unsigned long wsman_transport_get_timeout(WsManClient * cl)
{
	return cl->transport_timeout;
}

void wsman_transport_set_agent(WsManClient * cl, char *arg)
{
	cl->user_agent = arg;
}

char *wsman_transport_get_auth_method(WsManClient * cl)
{
	return cl->authentication.method;
}

int wsman_transport_get_verify_peer(WsManClient * cl)
{
	return cl->authentication.verify_peer;
}

int wsman_transport_get_verify_host(WsManClient * cl)
{
	return cl->authentication.verify_host;
}

char *wsman_transport_get_cafile(WsManClient * cl)
{
	return cl->authentication.cert_file;
}

void wsman_transport_set_proxy(WsManClient * cl, char *arg)
{
	cl->proxy_data.proxy = arg;
}

void wsman_transport_set_proxyauth(WsManClient * cl, char *arg)
{
	cl->proxy_data.proxy_auth = arg;
}

void wsman_transport_set_timeout(WsManClient * cl, unsigned long arg)
{
	cl->transport_timeout = arg;
}

char *wsman_transport_get_agent(WsManClient * cl)
{
	if (cl->user_agent)
		return cl->user_agent;
	else
		return DEFAULT_USER_AGENT;
}

void wsman_transport_set_auth_method(WsManClient * cl, char *arg)
{
	cl->authentication.method = arg;
}

void wsman_transport_set_verify_peer(WsManClient * cl, int arg)
{
	cl->authentication.verify_peer = arg;
}

void wsman_transport_set_verify_host(WsManClient * cl, int arg)
{
	cl->authentication.verify_host = arg;
}

void wsman_transport_set_cafile(WsManClient * cl, char *arg)
{
	cl->authentication.cert_file = arg;
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
