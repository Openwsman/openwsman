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

#include "wsman-soap-api.h"
#include "wsman-client-api.h"


char* wsman_make_action(char* uri, char* opName);

WsXmlDocH wsman_make_enum_message(WsContextH soap, char* op, 
		char* enumContext, char* resourceUri, char* url, actionOptions options);

WsXmlDocH wsman_enum_send_get_response(WsManClient *cl, char* op,
		char* enumContext, char* resourceUri, int max_elements, 
		actionOptions options);

extern void wsman_client_handler( WsManClient *cl, WsXmlDocH rqstDoc, void* user_data);

void wsman_client (WsManClient *cl, WsXmlDocH rqstDoc);

void initialize_action_options(actionOptions *op);
  
void destroy_action_options(actionOptions *op);     

WsXmlDocH ws_send_get_response(WsManClient *cl, WsXmlDocH rqstDoc,				
				unsigned long timeout);
				
int soap_submit_client_op(SoapOpH op, WsManClient *cl );        

void  wsman_add_selector_from_uri( WsXmlDocH doc, char *resourceUri);

void wsman_set_options_from_uri( char *resourceUri, actionOptions *options);

char *wsenum_get_enum_context(WsXmlDocH doc);

void wsman_add_fragment_transfer(  WsXmlDocH doc, char *fragment );

void wsman_add_namespace_as_selector( WsXmlDocH doc, char *_namespace);

void wsman_add_selectors_from_query_string(actionOptions *options, const char *query_string);

void wsman_add_selector_from_options( WsXmlDocH doc, 	actionOptions options);

WsXmlDocH wsman_build_envelope(WsContextH cntx, char* reply_to_uri, char* resource_uri,
        char* to_uri, actionOptions options);

void wsman_remove_query_string(char * resourceUri, char **result);

long long get_transfer_time(void);



WsXmlDocH wsman_build_envelope_from_response (WsManClient *cl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*WSMANCLIENT_H_*/
