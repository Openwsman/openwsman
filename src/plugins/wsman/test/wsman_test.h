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

#ifndef __WS_MAN_TEST_H__
#define __WS_MAN_TEST_H__

#include "wsman-declarations.h"
#include "wsman-xml-serializer.h"

#define WS_MAN_TEST_RESOURCE_URI		 XML_NS_OPENWSMAN"/test"


struct __WsManTestResult
{
	int result1;
	int result2;
};
typedef struct __WsManTestResult WsManTestResult;


// The resource is modeled as a struct
struct __WsManTest
{
	char* Simple;
	WsManTestResult Result;
	
};
typedef struct __WsManTest WsManTest;


// Service endpoint declaration
int WsManTest_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo);
int WsManTest_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo);
int WsManTest_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo);
WsManTest* WsManTest_Put_EP(WsContextH cntx);
WsManTest* WsManTest_Get_EP (WsContextH cntx);
int 
WsManTest_Subscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);
int WsManTest_Renew_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);
int WsManTest_UnSubscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);
int WsManTest_Evt_Pull_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);
int
WsManTest_EventPoll_EP(WsEventThreadContextH threadcntx);






SER_DECLARE_TYPE(WsManTest);
DECLARE_EP_ARRAY(WsManTest);
// DECLARE_SELECTOR_ARRAY(WsManTest);


void get_endpoints(void *self, void **data);
int init (void *self, void **data );
void cleanup( void *self, void *data );
void set_config( void *self, dictionary *config );
#endif //__WS_MAN_TEST_H__
