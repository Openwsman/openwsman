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
 * @author Zijiang Yang
 * @author Anas Nashif
 * @author Eugene Yarmosh
 */
 
#ifndef WS_SOAP_EVENTING_H
#define WS_SOAP_EVENTING_H

#include <stdio.h>
#include "glib.h"

#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "ws_dispatcher.h"
#include "wsman-client.h"

#include "xml_api_generic.h"
#include "xml_serializer.h"


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

#define WSE_DELIVERY_ACK_ACTION    "http://schemas.xmlsoap.org/ws/2005/06/management/Ack"

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

struct __EventingInfo
{
	SoapH soap;

        WsManClient *client;

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

/* -------------- API Routines --------------- */
EventingH wse_initialize_client(SoapH soap, WsManClient *client, char* managerUrl);
EventingH wse_initialize_server(SoapH soap, WsManClient *client, char* managerUrl);
void wse_destroy(EventingH hEventing);
void wse_process(EventingH hEventing);
int wse_subscriber_destroy(WseSubscriberH hSubscriber);
int wse_subscribe(WseSubscriberH hSubscriber, unsigned long durationSecs);
int wse_unsubscribe(WseSubscriberH hSubscriber);
int wse_renew(WseSubscriberH hSubscriber, unsigned long durationSeconds);
WsXmlDocH wse_get_status(WseSubscriberH hSubscriber);
WsePublisherH wse_publisher_initialize(EventingH hEventing, int actionCount, char **actionList, void *proc, void *data);
int wse_send_notification(WsePublisherH hPub, WsXmlDocH event, char *userNsUri, char *replyTo);
WseSubscriberH wse_subscriber_initialize(EventingH hEventing, int actionCount, char **actionList, 
                                         char *publisherUrl, char *subscriberUrl, void (*procNotification)(void *, WsXmlDocH), 
                                         void (*procSubscriptionEnd)(void *, WsXmlDocH), void *data);
int wse_renew_endpoint(SoapOpH op, void *data);
int wse_unsubscribe_endpoint(SoapOpH op, void *data);
int wse_subscribe_endpoint(SoapOpH op, void *data);
int wse_sink_endpoint(SoapOpH op, void *data);
int wse_get_status_endpoint(SoapOpH op, void *data);
int wse_subscription_end_endpoint(SoapOpH op, void *data);
int wse_publisher_destroy(WsePublisherH hPub, char *status, char *reason);
int wse_dont_unsubscribe_on_destroy(WseSubscriberH hSubscriber);
void wse_enum_sinks(EventingH hEventing, unsigned long (*proc)(void *data, char *id, char *url, 
                    unsigned long tm, void *reserved), void *data);
void wse_scan(EventingInfo *e, int enforceUnsubscribe);

extern int g_notify_connection_status;


/* -------------- Internal Routines --------------- */
void make_eventing_endpoint(EventingInfo *e, SoapServiceCallback endPointProc, SoapServiceCallback validateProc, char *opName);
void populate_string_list(DL_List *list, int count, char **strs);
int is_sink_expired(RemoteSinkInfo *sink);
void destroy_remote_sinks(EventingInfo *e, char *_status, char *_reason, int enforceDestroy);
int is_sink_for_notification(RemoteSinkInfo *sink, char *action);
int is_sink_for_publisher(RemoteSinkInfo *sink, PublisherInfo *pub);
WsXmlDocH build_notification(WsXmlDocH event, RemoteSinkInfo *sink, char *action, char *replyTo);
void populate_list_with_strings(char *buf, DL_List *list);
RemoteSinkInfo *make_remote_sink_info(EventingInfo *e, WsXmlDocH doc);
void destroy_remote_sink(RemoteSinkInfo *sink);
void destroy_publisher(PublisherInfo *pub);
void destroy_local_subscriber(LocalSubscriberInfo *sub);
void destroy_eventing_info(EventingInfo *e);
int is_filter_supported(EventingInfo *e, RemoteSinkInfo *sink);
RemoteSinkInfo *find_remote_sink(EventingInfo *e, WsXmlDocH doc);
LocalSubscriberInfo *find_subscriber_by_local_id(EventingInfo *e, WsXmlDocH doc);
LocalSubscriberInfo *find_subscriber(EventingInfo *e, char *action, LocalSubscriberInfo *sub);
void eventing_lock(EventingInfo *e);
void eventing_unlock(EventingInfo *e);
void add_eventing_action(EventingInfo *e, WsXmlNodeH node, char *opName);
void add_eventing_epr(EventingInfo *e, WsXmlNodeH xmlNode, char *urlName, char *url, char *idName, char *idNsUri, char *id);
void add_filters(WsXmlNodeH xmlNode, char *ns, DL_List *list, char *dialect);
WsXmlDocH build_eventing_request(EventingInfo *e, char *opName, LocalSubscriberInfo *sub, unsigned long durationSecs);
void add_epr_to_header(EventingInfo *e, WsXmlNodeH header, WsXmlNodeH epr);
WsXmlDocH build_eventing_response(EventingInfo *e, char *opName, RemoteSinkInfo *sink, char *relatesTo, char *status, char *reason);
int is_eventing_op_name(char *opName);
WsXmlDocH send_eventing_request(EventingInfo *e, WsXmlDocH rqst, char *url, unsigned long tm, unsigned long flags);
void send_eventing_response(EventingInfo *e, SoapOpH op, WsXmlDocH doc);
/*
WsXmlDocH build_get_metadata_response(SoapH soap, WsXmlDocH base, char *relatesTo);
int get_metadata_request_endpoint(SoapOpH op, void *data);
int set_metadata_request_endpoint(SoapH soap, WsXmlDocH base);
WsXmlDocH build_mex_get_metadata_request(SoapH soap);
WsXmlDocH mex_get_metadata(SoapH soap, char *url);
*/

#endif //WS_SOAP_EVENTING_H
