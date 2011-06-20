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
 * @author Vadim Revyakin
 * @author Anas Nashif
 */

#ifndef WSMAN_CLIENT_TRANSPORT_H_
#define WSMAN_CLIENT_TRANSPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "wsman-client-api.h"


#ifdef PACKAGE_STRING
#define DEFAULT_USER_AGENT PACKAGE_STRING
#else
#define DEFAULT_USER_AGENT "Openwsman"
#endif

#define _WS_NO_AUTH "No Auth"
#define _WS_BASIC_AUTH "Basic"
#define _WS_DIGEST_AUTH "Digest"
#define _WS_PASS_AUTH "Passport"
#define _WS_NTLM_AUTH "NTLM"
#define _WS_GSSNEGOTIATE_AUTH "GSS-Negotiate"

int wsman_send_request(WsManClient *cl, WsXmlDocH request);

/*
 * Set callback function to ask for username/password on authentication failure (http-401 returned)
 * If the callback returns an empty (or NULL) username, authentication is aborted.
 */

extern void wsmc_transport_set_auth_request_func(WsManClient *cl,
        wsman_auth_request_func_t f);


extern int wsman_is_auth_method(WsManClient *cl, int method);

extern int wsmc_transport_init(WsManClient *cl, void *arg);

extern void wsman_transport_close_transport(WsManClient *cl);

extern void wsmc_transport_fini(WsManClient *cl);

extern void   wsman_transport_set_agent(WsManClient *cl, const char *agent);
extern char * wsman_transport_get_agent (WsManClient *cl);

extern void wsman_transport_set_userName(WsManClient *cl, char *user_name);
extern void wsman_transport_set_password(WsManClient *cl, char *password);

extern void wsman_transport_set_proxy_username(WsManClient *cl, char *proxy_username );
extern void wsman_transport_set_proxy_password(WsManClient *cl, char *proxy_password );

extern void   wsman_transport_set_auth_method(WsManClient *cl, const char *am);
extern char * wsman_transport_get_auth_method (WsManClient *cl);

extern char *wsmc_transport_get_auth_name(wsman_auth_type_t auth);

extern  wsman_auth_type_t wsmc_transport_get_auth_value(WsManClient *cl);

char *wsman_transport_get_last_error_string(WS_LASTERR_Code err);

extern void          wsman_transport_set_timeout(WsManClient *cl, unsigned long timeout);
extern unsigned long wsman_transport_get_timeout(WsManClient *cl);

extern void wsman_transport_set_verify_peer(WsManClient *cl, unsigned int value);
extern unsigned int  wsman_transport_get_verify_peer(WsManClient *cl);

extern void wsman_transport_set_verify_host(WsManClient *cl, unsigned int value);
extern unsigned int  wsman_transport_get_verify_host(WsManClient *cl);

extern void wsman_transport_set_crlcheck(WsManClient * cl, unsigned int value);
extern unsigned int wsman_transport_get_crlcheck(WsManClient * cl);

extern void wsman_transport_set_crlfile(WsManClient *cl, const char *arg);
extern char *wsman_transport_get_crlfile(WsManClient *cl);

extern void  wsman_transport_set_proxy(WsManClient *cl, const char *proxy);
extern char *wsman_transport_get_proxy(WsManClient *cl);

extern void  wsman_transport_set_proxyauth(WsManClient *cl, const char *pauth);
extern char *wsman_transport_get_proxyauth(WsManClient *cl);

extern void  wsman_transport_set_cainfo(WsManClient *cl, const char *cainfo);
extern char *wsman_transport_get_cainfo(WsManClient *cl);

extern void wsman_transport_set_certhumbprint(WsManClient *cl, const char *arg);
extern char *wsman_transport_get_certhumbprint(WsManClient *cl);

extern void  wsman_transport_set_capath(WsManClient *cl, const char *capath);
extern char *wsman_transport_get_capath(WsManClient *cl);

extern void  wsman_transport_set_caoid(WsManClient *cl, const char *oid);
extern char *wsman_transport_get_caoid(WsManClient *cl);

#ifdef _WIN32
extern void wsman_transport_set_calocal(WsManClient *cl, BOOL local);
extern BOOL wsman_transport_get_calocal(WsManClient *cl);
#endif

extern void  wsman_transport_set_cert(WsManClient *cl, const char *cert);
extern char *wsman_transport_get_cert(WsManClient *cl);

extern void  wsman_transport_set_key(WsManClient *cl, const char *key);
extern char *wsman_transport_get_key(WsManClient *cl);

#ifdef DEBUG_VERBOSE
long long get_transfer_time(void);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* WSMAN_CLIENT_TRANSPORT_H_ */

