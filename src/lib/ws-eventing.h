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
 * @author Eugene Yarmosh
 */
 
#ifndef WS_SOAP_EVENTING_H
#define WS_SOAP_EVENTING_H

struct __EventingInfo
{
	SoapH soap;

	char* managerUrl;

	DL_List publisherList;

	DL_List localSubscriberList;
	
	DL_List remoteSinkList;
};
typedef struct __EventingInfo EventingInfo;

struct __PublisherInfo
{
	DL_Node node;
	EventingInfo* eventingInfo;
	DL_List actionList;
//	char* subcriptionManagerUrl;
};
typedef struct __PublisherInfo PublisherInfo;

struct __LocalSubscriberInfo
{
	DL_Node node;
	
	EventingInfo* eventingInfo;
	DL_List actionList;
	char* subscribeToUrl;
	char* subscriberUrl;
	char* uuid;

	WsXmlDocH doc;
	unsigned long sendRenewTicks;
	char* identifier;
	//WsXmlNodeH subscriptionManager;

	void (*procNotification)(void*, WsXmlDocH);
	void (*procSubscriptionEnd)(void*, WsXmlDocH);
	void* data;
};
typedef struct __LocalSubscriberInfo LocalSubscriberInfo;


struct __RemoteSinkInfo
{
	DL_Node node;

	EventingInfo* eventingInfo;
	DL_List actionList;

	WsXmlDocH doc;
	WsXmlNodeH notifyTo;
	WsXmlNodeH endTo;

	char* uuidIdentifier;
	unsigned long durationSeconds;
	unsigned long lastSetTicks;
};
typedef struct __RemoteSinkInfo RemoteSinkInfo;





#endif //WS_SOAP_EVENTING_H
