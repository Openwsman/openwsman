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

#include "wsman_config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"


#include "u/libu.h"

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-client-api.h"
#include "wsman-xml-serializer.h"

#include "wsman-soap-envelope.h"
#include "wsman-soap-message.h"

#include "wsman_test.h"



WsManTest g_WsManTest1 = { "Simple Test" , { 0, 0 } };
WsManTest g_WsManTest2 = { "Simple Test 1" , {0, 0}};
WsManTest g_WsManTest3 = { "Simple Test 2" , { 0, 0} };

WsManTest* g_WsManTestArr[2] = 
 {
 	&g_WsManTest2, &g_WsManTest3
 };

WsManTest* WsManTest_Get_EP(WsContextH cntx)
{
    debug ("Test Get Endpoint Called"); 
    return &g_WsManTest1;
}

WsManTest* WsManTest_Put_EP(WsContextH cntx)
{
    debug( "Put Endpoint Called"); 
    return &g_WsManTest1;
}

int WsManTest_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    debug( "Enumerate Endpoint Called");
    enumInfo->totalItems = 2;   
    return 0;
}

int WsManTest_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    debug( "Release Endpoint Called");    
    return 0;
}

int WsManTest_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{ 
    debug( "Pull Endpoint Called"); 
    if ( enumInfo->index >= 0 && enumInfo->index < 2 ){
        enumInfo->pullResultPtr = g_WsManTestArr[enumInfo->index];
    } else {    	
        enumInfo->pullResultPtr = NULL;
    }

    return 0;
}
#ifdef ENABLE_EVENTING_SUPPORT
int
WsManTest_EventPoll_EP(WsEventThreadContextH threadcntx)
{
	int retval = 0;
	WsNotificationInfoH notificationinfo = u_malloc(sizeof(*notificationinfo));
	if(notificationinfo == NULL) return -1;
	notificationinfo->headerOpaqueData = ws_xml_create_doc( XML_NS_OPENWSMAN"/test", "EventTopics");
	WsXmlNodeH node = ws_xml_get_doc_root(notificationinfo->headerOpaqueData);
	if(node) {
		ws_xml_set_node_text(node, "openwsman.event.test");
	}
	notificationinfo->EventAction = u_strdup(XML_NS_OPENWSMAN"/EventReport");
	notificationinfo->EventContent = ws_xml_create_doc( XML_NS_OPENWSMAN"/test", "TestReport");
	if(notificationinfo->EventContent == NULL) return retval;
	node = ws_xml_get_doc_root(notificationinfo->EventContent);
	time_t timest = time(0);
	struct tm tm;
	localtime_r(&timest, &tm);
	ws_xml_add_child_format(node, XML_NS_OPENWSMAN"/test", "EventTime","%u-%u%u-%u%uT%u%u:%u%u:%u%u",
			tm.tm_year + 1900, (tm.tm_mon + 1)/10, (tm.tm_mon + 1)%10,
			tm.tm_mday/10, tm.tm_mday%10, tm.tm_hour/10, tm.tm_hour%10,
			tm.tm_min/10, tm.tm_min%10, tm.tm_sec/10, tm.tm_sec%10);
	EventPoolOpSetH opset = threadcntx->soap->eventpoolOpSet;
	if(threadcntx->subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_PULL)
		retval = opset->addpull(threadcntx->subsInfo->subsId, notificationinfo);
	else
		retval = opset->add(threadcntx->subsInfo->subsId, notificationinfo);
	if(retval) {
		u_free(notificationinfo->EventAction);
		ws_xml_destroy_doc(notificationinfo->EventContent);
		ws_xml_destroy_doc(notificationinfo->headerOpaqueData);
		u_free(notificationinfo);
	}
	return 0;
}

int 
WsManTest_Subscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	debug("Test Subscribe Call");
	int retval = 0;
	/* TODO: create indication filter here and something else necessary */
	subsInfo->eventpoll = WsManTest_EventPoll_EP;

	return retval;
}

int WsManTest_Renew_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	debug("CIM Subscription");
	int retval = 0;
	/* 
	 * TODO: create indication filter here and something else necessary
	 */
	 subsInfo->eventpoll = WsManTest_EventPoll_EP;
	 

	return retval;
}

int WsManTest_UnSubscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData)
{
	return 0;
}

#endif
