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
 

#ifndef WS_SOAP_EVENTING_API_H
#define WS_SOAP_EVENTING_API_H

#define WSE_RESPONSE_TIMEOUT			20000

#define WSE_SUBSCRIBE					"Subscribe"	
#define WSE_SUBSCRIBE_RESPONSE			"SubscribeResponse"	
#define WSE_RENEW						"Renew"	
#define WSE_RENEW_RESPONSE				"RenewResponse"	
#define WSE_GET_STATUS					"GetStatus"	
#define WSE_GET_STATUS_RESPONSE			"GetStatusResponse"	
#define WSE_UNSUBSCRIBE					"Unsubscribe"	
#define WSE_UNSUBSCRIBE_RESPONSE		"UnsubscribeResponse"	
#define WSE_SUBSCRIPTION_END			"SubscriptionEnd"	

#define WSE_EXPIRES						"Expires"
#define WSE_END_TO						"EndTo"
#define WSE_NOTIFY_TO					"NotifyTo"
#define WSE_DELIVERY					"Delivery"
#define WSE_FILTER						"Filter"
#define WSE_DIALECT						"Dialect"
#define WSE_STATUS						"Status"
#define WSE_REASON						"Reason"
#define WSE_SUBSCRIPTION_MANAGER		"SubscriptionManager"
#define WSE_IDENTIFIER					"Identifier"

#define WSE_DELIVERY_FAILURE			"DeliveryFailure"
#define WSE_SOURCE_SHUTTING_DOWN		"SourceShuttingDown"
#define WSE_SOURCE_CANCELING			"SourceCanceling"


#define WSE_DIALECT_ACTION	"http://schemas.xmlsoap.org/ws/2004/08/devprof/Action"

#define WSE_DP_SUB_ID					"SubId"

#define WS_PUB_ADD			1
#define WS_PUB_CHECK		0
#define WS_PUB_REMOVE		-1


struct __EventingH
{
	int __undefined;
};
typedef struct __EventingH* EventingH;

struct __WsePublisherH
{
	int __undefined;
};
typedef struct __WsePublisherH* WsePublisherH;

struct __WseSubscriberH
{
	int __undefined;
};
typedef struct __WseSubscriberH* WseSubscriberH;


typedef int (*SubMatchProc)(WsePublisherH, void*, WsXmlDocH, WsXmlDocH*, int);

extern EventingH ws_initialize(SoapH soap, char* managerUrl);
extern void wse_destroy(EventingH hEventing);
extern void wse_process(EventingH hEventing);

extern WsePublisherH wse_publisher_initialize(EventingH hEventing,
								int actionCount, // if <= 0 zero terminated
								char** actionList,
//								char* subcriptionManagerUrl,
								SubMatchProc proc,
								void* data);
extern int wse_publisher_destroy(WsePublisherH hPub, char* status, char* reason);
extern int wse_send_notification(WsePublisherH hPub, 
							   WsXmlDocH notification, 
							   char* userNsUri);


extern WseSubscriberH wse_subscriber_initialize(EventingH hEventing,
								int actionCount,
								char** actionList,
								char* publisherUrl,
								char* subscriberUrl,
								void (*procNotification)(void*, WsXmlDocH),
								void (*procSubscriptionEnd)(void*, WsXmlDocH),
								void* data);
extern int wse_subscriber_destroy(WseSubscriberH hSubscriber);
extern int wse_dont_unsubscribe_on_destroy(WseSubscriberH hSubscriber);
extern void	wse_enum_sinks(EventingH hEventing, 
						 unsigned long (*proc)(void* data, 
											  char* id,
											  char* url,
											  unsigned long tm,
											  void* reserved),
						 void* data);


extern int wse_subscribe(WseSubscriberH hSubscriber, unsigned long durationSeconds);
extern int wse_unsubscribe(WseSubscriberH hSubscriber);
extern int wse_renew(WseSubscriberH hSubscriber, unsigned long durationSeconds);
extern WsXmlDocH wse_getstatus(WseSubscriberH hSubscriber);

// TBD: ??? move to separate file
extern int set_metadata_request_endpoint(SoapH soap, WsXmlDocH base);
extern WsXmlDocH mex_get_metadata(SoapH soap, char* url);


#endif //WS_SOAP_EVENTING_API_H
