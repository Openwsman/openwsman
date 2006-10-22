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
 */

#ifndef WSMAN_CLIENT_TRANSPORT_H_
#define WSMAN_CLIENT_TRANSPORT_H_

#include "u/libu.h"

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-client.h"

// FIXME, Duplicate?
// Possible authentication methods
#define AUTH_BASIC      0
#define AUTH_DIGEST     1
#define AUTH_NTLM       2
#define AUTH_MAX        3


typedef enum {
    WS_NO_AUTH,
    WS_BASIC_AUTH,
    WS_DIGEST_AUTH,
    WS_NTLM_AUTH,
} ws_auth_type_t;


typedef void (*ws_auth_request_func_t)(ws_auth_type_t, char **, char **);
extern void ws_client_transport_set_auth_request_func(ws_auth_request_func_t);
extern char *ws_client_transport_get_auth_name(ws_auth_type_t auth);

extern int wsman_client_transport_init(void *);
extern void wsman_client_handler( WsManClient *cl, WsXmlDocH rqstDoc, void* user_data);
extern void wsman_client_transport_fini(void);

extern int wsman_is_auth_method(int method);

#endif  /* WSMAN_CLIENT_TRANSPORT_H_ */

