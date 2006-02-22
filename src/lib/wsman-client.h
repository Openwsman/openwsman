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
  int    verifyMode;
  char * trustStore;
  char * certFile;
  char * keyFile;
} WsManCredentialData;


typedef struct _WsManClientFT 
{
	/**
	 * Transfer Get
	 */	
	WsXmlDocH (*Get)(WsManClient* cl, char* resourceUri);
	
	/**
	 * Transfer Put
	 */
	GList *(*Enumerate)(WsManClient* cl, char* resourceUri, int count);
	 	 	 	 	 
	
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


WsXmlDocH Get(        
        WsManClient *cl,
        char *resourceUri);

GList *Enumerate(        
        WsManClient *cl,
        char *resourceUri,
        int count);



WsXmlDocH wsman_make_enum_message(WsContextH soap,
        char* op,
        char* enumContext,
        char* resourceUri,
        char* url);

WsXmlDocH wsman_enum_send_get_response(
        WsManClient *cl,
        char* op,
        char* enumContext,
        char* resourceUri);


WsManConnection *initConnection(WsManClientData *cld);

typedef void (*WsmanClientFn) (WsManClient *cl,                           
                           	 WsXmlDocH rqstDoc,
                           	 gpointer user_data);

guint
wsman_client_add_handler (WsmanClientFn fn,                     
                      gpointer user_data);

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
char *wsman_remove_query_string(char * resourceUri);

	



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*WSMANCLIENT_H_*/
