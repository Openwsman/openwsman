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
 * @author Praveen K Paladugu 
 */
#include "wsman-declarations.h"
#include "wsman-xml-serializer.h"

#define XML_REDIRECT_NS    "http://dummy.com/wbem/wscim/1/cim-schema/2"


#define ENABLE_EVENTING_SUPPORT 0 

struct  __RedirectResult
{
	int result;
};
typedef struct __RedirectResult Redirect;

// Service endpoint declaration
int Redirect_Enumerate_EP(WsContextH cntx,
                        WsEnumerateInfo* enumInfo,
                        WsmanStatus *status, void *opaqueData);

int Redirect_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo);

int Redirect_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo);

int Redirect_Put_EP(WsContextH cntx);

Redirect  Redirect_Get_EP (WsContextH cntx);

/*int Redirect_Subscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int Redirect_Renew_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int Redirect_UnSubscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int Redirect_Evt_Pull_EP(WsContextH cntx,WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int Redirect_EventPoll_EP(WsEventThreadContextH threadcntx);
*/





SER_DECLARE_TYPE(Redirect);
DECLARE_EP_ARRAY(Redirect);


void get_endpoints(void *self, void **data);
int init (void *self, void **data );
void cleanup( void *self, void *data );
void set_config( void *self, dictionary *config );

char* get_remote_server();
char* get_remote_username();
char* get_remote_password();
char* get_remote_url_path();
char* get_remote_authentication_method();
char* get_remote_cim_namespace();
char* get_remote_cainfo();
int get_remote_server_port();
int get_remote_noverifypeer();
int get_remote_noverifyhost();
char *get_remote_sslkey();
char *get_remote_cl_cert();


