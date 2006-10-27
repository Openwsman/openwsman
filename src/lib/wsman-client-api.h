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

#ifndef WSMANCLIENT_API_H_
#define WSMANCLIENT_API_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



struct _WsManClient;
typedef struct _WsManClient WsManClient;


typedef struct _WsManClientStatus {
    unsigned int rc;
    char *msg;
} WsManClientStatus;


typedef struct clientData {
   char *hostName;
   unsigned int port;
   char *user;
   char *pwd;
   char *scheme;
   char *endpoint;
   unsigned int auth_method;
   int  status;
} WsManClientData;


typedef struct credentialData {
    char * certFile;
    char * keyFile;
	unsigned int verify_peer;
} WsManCredentialData;

typedef struct proxyData {
    char * proxy;
    char * proxy_auth;
} WsManProxyData;


struct _actionOptions {
    unsigned char       flags;
    char *              filter;
    char *              dialect;
    char *              fragment;
    char *              cim_ns;
	hash_t				*selectors;
    unsigned int        timeout;
    unsigned int        max_envelope_size;
};
typedef struct _actionOptions actionOptions;

struct _WsManConnection {
    char*	request;
    char*	response;
};
typedef struct _WsManConnection WsManConnection;

struct _WsManClient {
    void*          	        hdl;
    WsContextH				wscntx;
    WsManClientData      	data;
    WsManCredentialData  	certData;
    WsManConnection     	*connection;
	WsManProxyData			proxyData;
};

WsManClient*
wsman_connect( WsContextH wscntxt,
        const char *hostname,
        const int port,
        const char *path,
        const char *scheme,
        const char *username,
        const char *password);



WsXmlDocH wsman_identify(WsManClient *cl, actionOptions options);

WsXmlDocH ws_transfer_get(WsManClient *cl, char *resourceUri,
		actionOptions options); 

WsXmlDocH ws_transfer_put(WsManClient *cl, char *resourceUri,
		hash_t *prop, actionOptions options);

WsXmlDocH ws_transfer_create(WsManClient *cl, char *resourceUri,
		hash_t *prop, actionOptions options);

WsXmlDocH wsenum_enumerate(WsManClient *cl, char *resourceUri,
		int max_elements, actionOptions options);

WsXmlDocH wsenum_pull(WsManClient *cl, char *resourceUri, 
		char *enumContext , int max_elements, actionOptions options);

WsXmlDocH wsenum_release(WsManClient *cl, char *resourceUri,
		char *enumContext , actionOptions options);

WsXmlDocH wsman_invoke(WsManClient *cl, char *resourceUri , char *action,
		hash_t *prop, actionOptions options);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*WSMANCLIENT_H_*/
