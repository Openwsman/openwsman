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

#ifndef WSMANCLIENT_H_
#define WSMANCLIENT_H_


#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#ifndef WIN32
#include <pthread.h>
#endif				/* // !WIN32 */

#include "u/buf.h"
#include "wsman-client-api.h"
#include "wsman-xml-serialize.h"
#define WSMAN_CLIENT_BUSY       0x0001

	struct _WsManConnection {
		u_buf_t *request;
		u_buf_t *response;
	};
	typedef struct _WsManConnection WsManConnection;


	typedef struct {
		char *hostname;
		unsigned int port;
		char *path;
		char *user;
		char *pwd;
		char *scheme;
		char *endpoint;
		unsigned int auth_method;
		long auth_set;
		int status;
	} WsManClientData;


	struct _WsManAuthData {
		char *cainfo;
		char *caoid;
		unsigned char certificatethumbprint[20];
#ifdef _WIN32
		BOOL calocal;
#endif
		char *capath;
		char *sslcert;
		char *sslkey;
		unsigned int verify_peer;
		unsigned int verify_host;
	        wsman_auth_request_func_t auth_request_func;
	        char *method;

		unsigned int crl_check;
		char *crl_file;
	};
	typedef struct _WsManAuthData WsManAuthData;

	struct _WsManProxyData {
		char *proxy;
		char *proxy_auth;
		char *proxy_username;
		char *proxy_password;
	};
	typedef struct _WsManProxyData WsManProxyData;

	struct _WsManClient {
		void *hdl;
		int flags;
		pthread_mutex_t mutex;
		WsSerializerContextH serctx;
		WsManClientData data;
		WsManConnection *connection;
		WsManAuthData authentication;
		WsManProxyData proxy_data;

#ifdef _WIN32
		void* session_handle;
		long lock_session_handle;
#endif

		long response_code;
		char *fault_string;
		WS_LASTERR_Code last_error;
		void *transport;
		char *content_encoding;
		char *cim_ns;
		unsigned long transport_timeout;
		char * user_agent;
		FILE *dumpfile;
		long initialized;
#ifndef _WIN32
		char *client_config_file;
#endif

	};




	int wsmc_lock(WsManClient * cl);
	void wsmc_unlock(WsManClient * cl);


#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* WSMANCLIENT_H_ */
