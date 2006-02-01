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
#ifndef WSMAN_CLIENT_H
#define WSMAN_CLIENT_H






#define ACTION_TRANSFER_GET 	1
#define ACTION_TRANSFER_PUT	2
#define ACTION_ENUMERATION 	3
#define ACTION_PRIVATE_CATCH    4


#define MY_ENCODING "UTF-8"
#define SOAP1_2_CONTENT_TYPE "application/soap+xml; charset=utf-8"


struct _WsClientContext {
    char *uri, *username, *password;
    WsContextH  wscntx;
};
typedef struct _WsClientContext WsClientContextH;

struct _WsProperties {
    char *key;
    char *value;   
};
typedef struct _WsProperties WsProperties;



char* wsman_make_action(char* uri, char* opName);

void text_output(char *body);


SoapH wsman_client_initialize(void);
WsContextH wsman_client_create(void);


WsXmlDocH  _ws_send_get_response(WsClientContextH *ctx, WsXmlDocH rqstDoc, char* url);



int wsman_transfer_get (
        char *url,
        WsClientContextH *ctx,
        char *resourceUri
        );
        
 
WsXmlDocH wsman_make_enum_message(WsContextH soap,
        char* op,
        char* enumContext,
        char* resourceUri,
        char* url);

WsXmlNodeH wsman_enum_send_get_response(
        WsClientContextH *ctx,
        char* op,
        char* enumContext,
        char* resourceUri,
        char* url);

int  wsman_transfer_put(
	char* url, 
	WsClientContextH *ctx, 
	char *resourceUri, 
	WsProperties *properties);
	
int  wsman_enumeration(char* url,
        WsClientContextH *ctx,
        char *resourceUri,
        int count);
        
int  wsman_private_catch (
        char *url,
        WsClientContextH *ctx,
        char *resourceUri
        );
        
               
#endif // WSMAN_CLIENT_H
