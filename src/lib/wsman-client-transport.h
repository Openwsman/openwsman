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

#include "u/libu.h"
#include "wsman-client-api.h"



#define DEFAULT_USER_AGENT PACKAGE_STRING

int wsman_send_request(WsManClient *cl, WsXmlDocH request);

extern void wsman_client_transport_set_auth_request_func(WsManClient *cl, 
        wsman_auth_request_func_t f);


extern int wsman_is_auth_method(WsManClient *cl, int method);

extern int wsman_client_transport_init(WsManClient *cl, void *arg);

extern void wsman_client_transport_fini(WsManClient *cl);

extern char *wsman_transport_get_proxy(WsManClient *cl);

extern unsigned long wsman_transport_get_timeout(WsManClient *cl);

extern char *wsman_transport_get_proxyauth(WsManClient *cl);

extern char * wsman_transport_get_agent (WsManClient *cl);

extern char * wsman_transport_get_auth_method (WsManClient *cl);

extern int wsman_transport_get_verify_peer (WsManClient *cl);

extern int wsman_transport_get_verify_host (WsManClient *cl);

extern char *wsman_client_transport_get_auth_name(wsman_auth_type_t auth);

extern int wsman_client_transport_get_auth_value(WsManClient *cl);

extern char *wsman_transport_get_cafile(WsManClient *cl);

char *wsman_transport_get_last_error_string(WS_LASTERR_Code err);

extern void wsman_transport_set_verify_peer(WsManClient *cl, int value);

extern void wsman_transport_set_verify_host(WsManClient *cl, int value);

extern void wsman_transport_set_proxy(WsManClient *cl, char *proxy);

extern void wsman_transport_set_proxyauth(WsManClient *cl, char *pauth);

extern void wsman_transport_set_timeout(WsManClient *cl, unsigned long timeout);

extern void wsman_transport_set_agent(WsManClient *cl, char *agent);

extern void wsman_transport_set_auth_method(WsManClient *cl, char *am);

extern void wsman_transport_set_cafile(WsManClient *cl, char *caf);

extern void  wsman_transport_set_agent (WsManClient *cl, char *arg);

extern void wsman_transport_close_transport(WsManClient *cl);

#ifdef DEBUG_VERBOSE
long long get_transfer_time(void);
#endif


#endif  /* WSMAN_CLIENT_TRANSPORT_H_ */

