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


#ifndef __CIM_DATA_H__
#define __CIM_DATA_H__

#include "wsman-declarations.h"
#include "wsman-xml-serializer.h"



// The resource is modeled as a struct
struct __CimResource
{
	char* xml;
};
typedef struct __CimResource CimResource;


// Service endpoint declaration

int CimResource_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData);

int CimResource_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData);

int CimResource_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo,
		WsmanStatus *status,
		void *opaqueData);

int CimResource_Get_EP(SoapOpH op, void* appData, void *opaqueData);

int CimResource_Custom_EP(SoapOpH op, void* appData, void *opaqueData);

int CimResource_Put_EP(SoapOpH op, void* appData, void *opaqueData);

int CimResource_Create_EP(SoapOpH op, void* appData, void *opaqueData);

int CimResource_Delete_EP(SoapOpH op, void* appData, void *opaqueData);

int CimResource_Subscribe_EP(WsContextH cntx, WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int CimResource_Renew_EP(WsContextH cntx, WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int CimResource_UnSubscribe_EP(WsContextH cntx, WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);


//If you want to support poll, implement it.
int CimResource_EventPoll_EP(WsEventThreadContextH cntx);
//Subscription cancel routine
int CimResource_SubscriptionCancel_EP(WsEventThreadContextH cntx);
SER_DECLARE_TYPE(CimResource);
DECLARE_EP_ARRAY(CimResource);

void get_endpoints(void *self, void **data);

int init (void *self, void **data );

void cleanup( void *self, void *data );

void set_config( void *self, dictionary *config );

char *get_cim_namespace(void);
char *get_cim_client_frontend(void);
char *get_cim_indication_SourceNamespace(void);

hash_t* get_vendor_namespaces(void);
char *get_cim_host(void);
char *get_cim_port(void);
char *get_server_port(void);
int get_omit_schema_optional(void);
#endif // __CIM_DATA_H__
