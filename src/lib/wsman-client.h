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
#endif /* __cplusplus */

struct _WsManClientEnc;
typedef struct _WsManClientEnc WsManClientEnc;

struct _WsManClient;
typedef struct _WsManClient WsManClient;

typedef struct _WsManStatus {
      unsigned int rc;
      char *msg;
} WsManClientStatus;

typedef struct clientData {
   char *hostName;
   int port;
   char *user;
   char *pwd;
   char *scheme;
   char *endpoint;
   int  status;
} WsManClientData;


typedef struct credentialData {
  char * certFile;
  char * keyFile;
} WsManCredentialData;


typedef struct _WsManClientFT 
{
         WsManClientStatus (*release)(WsManClient * cl);

	/**
	 * Transfer Get
	 */	
	WsXmlDocH (*get)(WsManClient* cl, char* resourceUri, actionOptions options);
        
	/**
	 * Transfer Put
	 */	
	WsXmlDocH (*put)(WsManClient* cl, char* resourceUri, GList *prop, actionOptions options);
	
	/**
	 * Enumerate
	 */
	WsXmlDocH (*wsenum_enumerate)(WsManClient* cl, char* resourceUri, int max_elements, actionOptions options);

	/**
	 * Pull
	 */
	WsXmlDocH (*wsenum_pull)(WsManClient* cl, char* resourceUri, char *enumContext, int max_elements, actionOptions options);
        
	/**
	 * Release
	 */
	WsXmlDocH (*wsenum_release)(WsManClient* cl, char* resourceUri, char *enumContext, actionOptions options);

	/**
	 * Transfer Create
	 */	
	WsXmlDocH (*create)(WsManClient* cl, char* resourceUri, GList *prop, actionOptions options);
	/**
	 * Invoke custom method
	 */	
	WsXmlDocH (*invoke)(WsManClient* cl, char* resourceUri, char *action,  GList *prop, actionOptions options);


	WsXmlDocH (*identify)(WsManClient* cl, actionOptions options);
        
	 	 	 	 	 
	
} WsManClientFT;


struct _WsManClient {
   void *hdl;
   WsManClientFT *ft;
};

struct _WsManConnection {
	char*	request;
	char*	response;
};
typedef struct _WsManConnection WsManConnection;

struct _WsManClientEnc {
   	WsManClient          	enc;
	WsContextH		wscntx;
   	WsManClientData      	data;
   	WsManCredentialData  	certData;
   	WsManConnection     	*connection;
};

char* wsman_make_action(char* uri, char* opName);


WsManClient *wsman_connect( 
		WsContextH wscntxt,
		const char *hostname,
		const int port,
		const char *scheme,
		const char *username,
		const char *password,		
		WsManClientStatus *rc);
WsManClient *wsman_connect_with_ssl( 
		WsContextH wscntxt,
		const char *hostname,
		const int port,
		const char *scheme,
		const char *username,
		const char *password,		
                const char * certFile, 
                const char * keyFile,
		WsManClientStatus *rc);


WsXmlDocH wsman_identify(WsManClient *cl, actionOptions options);
WsXmlDocH transfer_get(WsManClient *cl, char *resourceUri, actionOptions options); 
WsXmlDocH transfer_put(WsManClient *cl, char *resourceUri, GList *prop, actionOptions options);
WsXmlDocH transfer_create(WsManClient *cl, char *resourceUri, GList *prop, actionOptions options);
WsXmlDocH wsenum_enumerate(WsManClient *cl, char *resourceUri, int max_elements, actionOptions options);
WsXmlDocH wsenum_pull(WsManClient *cl, char *resourceUri, char *enumContext , int max_elements, actionOptions options);
WsXmlDocH wsenum_release(WsManClient *cl, char *resourceUri, char *enumContext , actionOptions options);
WsXmlDocH invoke(WsManClient *cl, char *resourceUri , char *action,  GList *prop, actionOptions options);

WsXmlDocH wsman_make_enum_message(WsContextH soap, char* op, char* enumContext, char* resourceUri, char* url, actionOptions options);

WsXmlDocH wsman_enum_send_get_response(WsManClient *cl, char* op, char* enumContext, char* resourceUri, 
        int max_elements, actionOptions options);

WsManConnection *initClientConnection(WsManClientData *cld);

typedef void (*WsmanClientFn) (WsManClient *cl,                           
                           	 WsXmlDocH rqstDoc,
                           	 gpointer user_data);

void wsman_client (WsManClient *cl, WsXmlDocH rqstDoc);

guint wsman_client_add_handler (WsmanClientFn fn, gpointer user_data);

void
wsman_client_remove_handler (guint id);

       
// TEMP

WsXmlDocH ws_send_get_response(WsManClient *cl, 
				WsXmlDocH rqstDoc,				
				unsigned long timeout);
				
int soap_submit_client_op(SoapOpH op, WsManClient *cl );        


char* wsman_add_selector_from_uri(
	WsManClient *cl, 
	WsXmlDocH doc, 
	char *resourceUri);


char *wsenum_get_enum_context(WsXmlDocH doc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*WSMANCLIENT_H_*/
