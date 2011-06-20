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
 * @author Sumeet Kukreja, Dell Inc.
 * @author Liang Hou
 */

#ifndef WSMAN_SOAP_H_
#define WSMAN_SOAP_H_

#include "u/hash.h"
#include "u/list.h"
#include "wsman-faults.h"
#include "wsman-soap-message.h"
#include "wsman-xml-api.h"
#include "wsman-filter.h"
#include "wsman-event-pool.h"
#include "wsman-subscription-repository.h"
#include "wsman-xml-serializer.h"

#define SOAP_MAX_RESENT_COUNT       10
#define PEDNING_EVENT_MAX_COUNT	10


#define WS_DISP_TYPE_MASK               0xffff

#define WS_DISP_TYPE_RAW_DOC            0
#define WS_DISP_TYPE_GET                1
#define WS_DISP_TYPE_PUT                2
#define WS_DISP_TYPE_CREATE         3
#define WS_DISP_TYPE_DELETE         4

#define WS_DISP_TYPE_ENUMERATE      5
#define WS_DISP_TYPE_PULL               6
#define WS_DISP_TYPE_RELEASE            7
#define WS_DISP_TYPE_UPDATE         8
#define WS_DISP_TYPE_GETSTATUS      9
#define WS_DISP_TYPE_COUNT              11
#define WS_DISP_TYPE_DIRECT_PULL         12
#define WS_DISP_TYPE_DIRECT_GET            13
#define WS_DISP_TYPE_GET_NAMESPACE  14
#define WS_DISP_TYPE_CUSTOM_METHOD  15
#define WS_DISP_TYPE_DIRECT_PUT            16
#define WS_DISP_TYPE_IDENTIFY           17
#define WS_DISP_TYPE_DIRECT_CREATE           18
#define WS_DISP_TYPE_DIRECT_DELETE           19
#define WS_DISP_TYPE_ENUM_REFINSTS           21
#define WS_DISP_TYPE_SUBSCRIBE		22
#define WS_DISP_TYPE_UNSUBSCRIBE		23
#define WS_DISP_TYPE_RENEW			24
#define WS_DISP_TYPE_EVT_PULL			25
#define WS_DISP_TYPE_ACK				26
#define WS_DISP_TYPE_PRIVATE                0xfffe



struct __dispatch_t;
typedef struct __dispatch_t *SoapDispatchH;

typedef SoapDispatchH (*DispatcherCallback) (WsContextH, void *, WsXmlDocH);

struct __SoapOp {
	unsigned        __undefined;
};
typedef struct __SoapOp *SoapOpH;

struct __Soap {
	/* do not move this field */
	pthread_mutex_t lockData;
	unsigned long   uniqueIdCounter;

	list_t         *inboundFilterList;
	list_t         *outboundFilterList;

	list_t         *dispatchList;
	list_t         *processedMsgIdList;

	pthread_mutex_t lockSubs; //lock for Subscription Repository
	char 			*uri_subsRepository; //URI of repository
	SubsRepositoryOpSetH subscriptionOpSet; //Function talbe of Subscription Repository
	EventPoolOpSetH eventpoolOpSet; //Function table of event source
	WsContextH      cntx;
	void           	*dispatcherData;
	DispatcherCallback dispatcherProc;
	void *listener;
};
typedef struct __Soap *SoapH;

struct _WsXmlDoc {
	void           *parserDoc;
	unsigned long   prefixIndex; // to enumerate not well known namespaces
};


struct _WS_CONTEXT_ENTRY {
	lnode_t        *node;
	unsigned long   size;
	unsigned long   options;
	char           *name;
};
typedef struct _WS_CONTEXT_ENTRY WS_CONTEXT_ENTRY;

struct _WS_CONTEXT {
	SoapH soap;
	unsigned long enumIdleTimeout;
	WsXmlDocH	indoc;
	hash_t *enuminfos;
	hash_t *entries;
	WsSerializerContextH serializercntx;
	list_t         	*subscriptionMemList; //memory Repository of Subscriptions
	/* to prevent user from destroying cntx he hasn't created */
	int             owner;
};

typedef struct __WsSubscribeInfo WsSubscribeInfo;
// EventThreadContext
struct __WsEventThreadContext {
	SoapH soap;
	WsSubscribeInfo *subsInfo;
	WsXmlDocH outdoc;
};
typedef struct __WsEventThreadContext * WsEventThreadContextH;

typedef void    (*WsProcType) (void);
struct __XmlSerializerInfo;
struct __WsDispatchEndPointInfo {
	/* put/get/create/delete rpc enumerate/release/pull/update/getstatus */
	unsigned long   flags;
	char           *rqstName;
	char           *respName;
	char           *inAction;
	char           *outAction;
	struct __XmlSerializerInfo *serializationInfo;
	WsProcType      serviceEndPoint;
	void           *data;
	struct __WsSelector *selectors;
};
typedef struct __WsDispatchEndPointInfo WsDispatchEndPointInfo;

struct __WsSupportedNamespaces {
	char           *ns;
	char           *class_prefix;
};
typedef struct __WsSupportedNamespaces WsSupportedNamespaces;


struct __WsDispatchInterfaceInfo {
	unsigned long   flags;
	char           *config_id;
	char           *version;
	char           *notes;
	char           *vendor;
	char           *displayName;
	char           *compliance;
	char           *actionUriBase;
	char           *wsmanResourceUri;
	void           *extraData;
	list_t         *namespaces;
	WsDispatchEndPointInfo *endPoints;
};
typedef struct __WsDispatchInterfaceInfo WsDispatchInterfaceInfo;
struct __DispatchToEpMap {
	SoapDispatchH   disp;
	WsDispatchEndPointInfo *ep;

};
typedef struct __DispatchToEpMap DispatchToEpMap;

struct __WsManDispatcherInfo {
	int             interfaceCount;
	int             mapCount;
	void           *interfaces;
	DispatchToEpMap map[1];
};
typedef struct __WsManDispatcherInfo WsManDispatcherInfo;



typedef struct __WsEnumerateInfo WsEnumerateInfo;

typedef int	(*WsEndPointSubscribe) (WsContextH,WsSubscribeInfo *, WsmanStatus *, void *);

typedef int	(*WsEndPointUnSubscribe) (WsContextH,WsSubscribeInfo *, WsmanStatus *, void *);

typedef int	(*WsEndPointRenew) (WsContextH,WsSubscribeInfo *, WsmanStatus *, void *);

typedef int     (*WsEndPointEnumerate) (WsContextH, WsEnumerateInfo *, WsmanStatus *, void *);

typedef int     (*WsEndPointPull) (WsContextH, WsEnumerateInfo *, WsmanStatus *, void *);

typedef int     (*WsEndPointRelease) (WsContextH, WsEnumerateInfo *, WsmanStatus *, void *);

typedef int     (*WsEndPointPut) (WsContextH, void *, void **, WsmanStatus *, void *);

typedef void   *(*WsEndPointGet) (WsContextH, WsmanStatus *, void *);

#define WSE_NOTIFICATION_DRAOPEVENTS 1
#define WSE_NOTIFICATION_HEARTBEAT 2
#define WSE_NOTIFICATION_NOACK 3
#define WSE_NOTIFICATION_EVENTS_PENDING 4

typedef int (*WsEndPointEventPoll) (WsEventThreadContextH);
typedef int (*WsEndPointSubscriptionCancel) (WsEventThreadContextH);

#define EUIDLEN		64

#define WSMAN_ENUMINFO_INWORK_FLAG	  0x000010
#define WSMAN_ENUMINFO_POLY_NONE	  0x000020
#define WSMAN_ENUMINFO_POLY_INCLUDE       0x000040
#define WSMAN_ENUMINFO_POLY_EXCLUDE       0x000080
#define WSMAN_ENUMINFO_EST_COUNT          0x000100
#define WSMAN_ENUMINFO_OPT          	  0x000200
#define WSMAN_ENUMINFO_EPR          	  0x000400
#define WSMAN_ENUMINFO_OBJEPR          	  0x000800
#define WSMAN_ENUMINFO_EXT          	  0x001000
#define WSMAN_ENUMINFO_ASSOC          	  0x020000
#define WSMAN_ENUMINFO_REF          	  0x040000
#define WSMAN_ENUMINFO_CQL          	  0x080000
#define WSMAN_ENUMINFO_WQL          	  0x100000
#define WSMAN_ENUMINFO_SELECTOR		0x200000
#define WSMAN_ENUMINFO_CIM_CONTEXT_CLEANUP	0x400000


struct __WsEnumerateInfo {
	unsigned long flags;
	char            enumId[EUIDLEN];
	unsigned long   timeStamp; // in msecs
	unsigned long   expires; // expiration time in msecs  since  the epoch
	unsigned int    totalItems;
	unsigned int    maxItems;
	unsigned int	maxsize; //max envelope size
	unsigned int             index;
	void           *enumResults;
	void           *pullResultPtr;
	void           *appEnumContext;
	WsmanAuth       auth_data;
	WsEndPointRelease releaseproc;
	char *		epr_to;
	char *		epr_uri;
	char *		encoding;
	void *		aux;
	void		*epr;
	filter_t	*filter;
};

#define WSMAN_SUBSCRIBEINFO_UNSUBSCRIBE 0x01
#define WSMAN_SUBSCRIBEINFO_RENEW 0x02
#define WSMAN_SUBSCRIBEINFO_BOOKMARK_DEFAULT	0x04
#define WSMAN_SUBSCRIBEINFO_MANAGER_STARTED 0x08
#define WSMAN_SUBSCRIPTION_CQL 0x10
#define WSMAN_SUBSCRIPTION_WQL 0x20
#define WSMAN_SUBSCRIPTION_SELECTORSET 0x40
#define WSMAN_SUBSCRIPTION_NOTIFICAITON_PENDING 0x80
#define WSMAN_SUBSCRIPTION_CANCELLED 0x100

#define WS_EVENT_DELIVERY_MODE_PUSH 1 /* http://schemas.xmlsoap.org/ws/2004/08/eventing/DeliveryModes/Push */
#define WS_EVENT_DELIVERY_MODE_PUSHWITHACK 2 /* http://schemas.dmtf.org/wbem/wsman/1/wsman/PushWithAck */
#define WS_EVENT_DELIVERY_MODE_EVENTS 3 /* "	http://schemas.dmtf.org/wbem/wsman/1/wsman/Events */
#define WS_EVENT_DELIVERY_MODE_PULL 4 /* http://schemas.dmtf.org/wbem/wsman/1/wsman/Pull */

#define WSMAN_SECURITY_PROFILE_HTTP_BASIC_TYPE 1
#define WSMAN_SECURITY_PROFILE_HTTP_DIGEST_TYPE 2
#define WSMAN_SECURITY_PROFILE_HTTPS_BASIC_TYPE 3
#define WSMAN_SECURITY_PROFILE_HTTPS_DIGEST_TYPE 4
#define WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_TYPE 5
#define WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_BASIC_TYPE 6
#define WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_DIGEST_TYPE 7
#define WSMAN_SECURITY_PROFILE_HTTPS_SPNEGO_KERBEROS_TYPE 8
#define WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_SPNEGO_KERBEROS_TYPE 9
#define WSMAN_SECURITY_PROFILE_HTTP_SPNEGO_KERBEROS_TYPE 10

struct __WsSubscribeInfo {
	pthread_mutex_t notificationlock;
	unsigned long flags;
	long heartbeatCountdown; //countdown of heartbeat, when it is 0, a heartbeat generated
	char            subsId[EUIDLEN];
	char *	soapNs;
	char *	uri;
	char *	cim_namespace; //CIM namespace
	void *	existingfilterOP; //Object path of existing filter class
	hash_t *	vendor_namespaces; //CIM vendor namespaces;
	char *	epr_notifyto; //A delivery destination for notification messages, using some delivery mode
	char *	username; // username for event sink
	char *	password; // password for event sink
	char *	certificate_thumbprint; //certificate thumbprint of event sink
	char * locale; // language code
	char * contentEncoding; //"UTF-8" or "UTF-16" or something else
	unsigned long expires;
	int	deliveryMode; /*The delivery mode to be used for notification messages sent in relation to this subscription.
	                                         Implied value is WS_EVENT_DELIVERY_MODE_PUSH,
	                                         which indicates that Push Mode delivery should be used. */
	int	deliveryAuthType; // security profile to use when making the connection to delivery events
	unsigned int	connectionRetryCount; // count of connection retry
	unsigned long connectionRetryinterval; //how long to wait between retries while trying to connect
	unsigned long heartbeatInterval; //Interval to send a heartbeart
	unsigned char eventSentLastTime; //To indicate whether an event is sent since last heartbeat
	WsXmlDocH bookmarkDoc;
	unsigned char bookmarksFlag; // whether bookmark is needed
	filter_t	*filter;
	WsmanAuth       auth_data;
	WsEndPointEventPoll eventpoll; // plugin related poll process
	WsEndPointSubscriptionCancel cancel; //plugin related subscription cancel routine
	WsXmlDocH templateDoc; //template notificaiton document
	WsXmlDocH heartbeatDoc; //Fixed heartbeat document
};


#if 0
enum __WsmanFilterDialect {
	WSMAN_FILTER_XPATH,
	WSMAN_FILTER_SELECTOR
};
typedef enum __WsmanFilterDialect WsmanFilterDialect;


enum __WsmanPolymorphismMode {
	INCLUDE_SUBCLASS_PROP = 1,
	EXCLUDE_SUBCLASS_PROP,
	POLYMORPHISM_NONE
};
typedef enum __WsmanPolymorphismMode WsmanPolymorphismMode;
#endif


typedef int     (*SoapServiceCallback) (SoapOpH, void *, void *);
struct __callback_t {
	lnode_t         node;
	              //dataBuf is passed to callback as data
	                SoapServiceCallback proc;
};
typedef struct __callback_t callback_t;

callback_t     *
make_callback_entry(SoapServiceCallback proc,
		    void *data,
		    list_t * list_to_add);



SoapH ws_soap_initialize(void);

void ws_set_context_enumIdleTimeout(WsContextH cntx,
                            unsigned long timeout);

void soap_destroy(SoapH soap);

SoapH ws_context_get_runtime(WsContextH hCntx);


int
wsman_register_interface(WsContextH cntx,
			 WsDispatchInterfaceInfo * wsInterface,
			 WsManDispatcherInfo * dispInfo);

int
wsman_register_endpoint(WsContextH cntx,
			WsDispatchInterfaceInfo * wsInterface,
			WsDispatchEndPointInfo * ep,
			WsManDispatcherInfo * dispInfo);


int ws_transfer_put_stub(SoapOpH op, void *appData, void *opaqueData);

int ws_transfer_delete_stub(SoapOpH op, void *appData, void *opaqueData);

int wsman_identify_stub(SoapOpH op, void *appData, void *opaqueData);

int wsenum_enumerate_stub(SoapOpH op, void *appData, void *opaqueData);

int ws_transfer_get_stub(SoapOpH op, void *appData, void *opaqueData);

int wsenum_pull_stub(SoapOpH op, void *appData, void *opaqueData);

int wsenum_pull_direct_stub(SoapOpH op, void *appData, void *opaqueData);

int wsenum_release_stub(SoapOpH op, void *appData, void *opaqueData);

int wse_subscribe_stub(SoapOpH op, void *appData, void *opaqueData);

int	wse_unsubscribe_stub(SoapOpH op, void *appData, void *opaqueData);

int	wse_renew_stub(SoapOpH op, void *appData, void *opaqueData);

int	wse_pull_stub(SoapOpH op, void *appData, void * opaqueData);

SoapOpH
soap_create_op(SoapH soap,
	       char *inboundAction,
	       char *outboundAction,
	       char *role,
	       SoapServiceCallback callbackProc,
	       void *callbackData,
	       unsigned long flags);
void            soap_destroy_op(SoapOpH op);
WsXmlDocH       soap_get_op_doc(SoapOpH op, int inbound);
WsXmlDocH       soap_detach_op_doc(SoapOpH op, int inbound);
int             soap_set_op_doc(SoapOpH op, WsXmlDocH doc, int inbound);
SoapH           soap_get_op_soap(SoapOpH op);



WsContextH      ws_create_context(SoapH soap);

void            ws_initialize_context(WsContextH hCntx, SoapH soap);

WsContextH      ws_create_runtime(list_t * interfaces);

WsContextH      ws_create_ep_context(SoapH soap, WsXmlDocH doc);

WsContextH      ws_get_soap_context(SoapH soap);

int             ws_destroy_context(WsContextH hCntx);

const void     *get_context_val(WsContextH hCntx, const char *name);

const void     *ws_get_context_val(WsContextH cntx, const char *name, int *size);

unsigned long   ws_get_context_ulong_val(WsContextH cntx, char *name);

int             ws_set_context_ulong_val(WsContextH cntx, char *name, unsigned long val);

int             ws_set_context_xml_doc_val(WsContextH cntx, char *name, WsXmlDocH val);

int             ws_remove_context_val(WsContextH hCntx, char *name);

hnode_t        *

create_context_entry(hash_t * h,
		     char *name,
		     void *val);

void            destroy_context_entry(WS_CONTEXT_ENTRY * entry);

int             wsman_fault_occured(WsmanMessage * msg);

WsmanKnownStatusCode wsman_find_httpcode_for_value(WsXmlDocH doc);

WsmanKnownStatusCode wsman_find_httpcode_for_fault_code(WsmanFaultCodeType faultCode);


WsXmlDocH
wsman_generate_fault(WsXmlDocH inDoc,
		     WsmanFaultCodeType faultCode,
		     WsmanFaultDetailType faultDetail,
		     char *fault_msg);
void
wsman_generate_fault_buffer( WsXmlDocH inDoc,
			     const char *encoding,
			    WsmanFaultCodeType faultCode,
			    WsmanFaultDetailType faultDetail,
			    char *fault_msg,
			    char **buf,
			    int *len);


void wsman_status_init(WsmanStatus * s);

int wsman_check_status(WsmanStatus * s);

void  wsman_timeouts_manager(WsContextH cntx, void *opaqueData);

void wsman_heartbeat_generator(WsContextH cntx, void *opaqueData);

WsEventThreadContextH ws_create_event_context(SoapH soap, WsSubscribeInfo *subsInfo, WsXmlDocH doc);

void * wse_notification_sender(void * thrdcntx);

void *wse_heartbeat_sender(void * thrdcntx);

void wse_notification_manager(void * cntx);

int outbound_addressing_filter(SoapOpH opHandle, void *data,
			       void *opaqueData);

int outbound_control_header_filter(SoapOpH opHandle, void *data,
				   void *opaqueData);

int soap_add_filter(SoapH soap, SoapServiceCallback callbackProc,
		    void *callbackData, int inbound);

WsmanMessage *wsman_get_msg_from_op(SoapOpH op) ;

unsigned long wsman_get_maxsize_from_op(SoapOpH op);

#endif				/* SOAP_API_H_ */
