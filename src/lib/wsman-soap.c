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
 * @author Liang Hou
 */

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#define _GNU_SOURCE

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-dispatcher.h"
#include "wsman-xml-serializer.h"
#include "wsman-xml-serialize.h"
#include "wsman-soap-envelope.h"
#include "wsman-faults.h"
#include "wsman-soap-message.h"

#include "wsman-client-api.h"
#include "wsman-client-transport.h"

/*    ENUMERATION  */
#define ENUM_EXPIRED(enuminfo, mytime) \
	((enumInfo->expires > 0) &&        \
	(enumInfo->expires > mytime))



/**
 * Calculate needed space for interface array with Endpoints
 * @param interfaces List of interfaces
 * @return Needed size of WsManDispatcherInfo
 */
static int
calculate_map_count(list_t * interfaces)
{
	int             count = 0;
	int             j;

	lnode_t        *node = list_first(interfaces);
	while (node) {
		WsDispatchInterfaceInfo *ifc =
			(WsDispatchInterfaceInfo *) node->list_data;
		for (j = 0; ifc->endPoints[j].serviceEndPoint != NULL; j++)
			count++;
		node = list_next(interfaces, node);
	}

	return (list_count(interfaces) * sizeof(WsManDispatcherInfo))
		+ (count * sizeof(DispatchToEpMap));
}

/**
 * Register Dispatcher
 * @param cntx Context
 * @param proc Dispatcher Callback
 * @param data Callback data
 */
static void
ws_register_dispatcher(WsContextH cntx, DispatcherCallback proc, void *data)
{
	SoapH soap = ws_context_get_runtime(cntx);
	if (soap) {
		soap->dispatcherProc = proc;
		soap->dispatcherData = data;
	}
	return;
}

static int
set_context_val(WsContextH cntx,
		char *name,
		void *val,
		int size,
		int no_dup,
		unsigned long type)
{
	int retVal = 1;
	debug("Setting context value: %s", name);
	if (cntx && name) {
		void *ptr = val;

		if (!no_dup) {
			if (val && (ptr = u_malloc(size))) {
				memcpy(ptr, val, size);
			}
		}
		if (ptr || val == NULL) {
			u_lock(cntx->soap);
			ws_remove_context_val(cntx, name);
			if (create_context_entry(cntx->entries, name, ptr)) {
				retVal = 0;
			}
			u_unlock(cntx->soap);
		}
	} else {
		error("error setting context value.");
	}
	return retVal;
}


static void
free_hentry_func(hnode_t * n, void *arg)
{
	u_free(hnode_getkey(n));
	u_free(n);
}


static void
remove_locked_enuminfo(WsContextH cntx,
                       WsEnumerateInfo * enumInfo)
{
	u_lock(cntx->soap);
	if (!(enumInfo->flags & WSMAN_ENUMINFO_INWORK_FLAG)) {
		error("locked enuminfo unlocked");
		u_unlock(cntx->soap);
		return;
	}
	hash_delete_free(cntx->enuminfos,
	             hash_lookup(cntx->enuminfos, enumInfo->enumId));
	u_unlock(cntx->soap);
}

#ifdef ENABLE_EVENTING_SUPPORT
static void wsman_expiretime2xmldatetime(unsigned long expire, char *str)
{
	time_t t = expire;
	struct tm tm;
	int gmtoffset_hour;
	int gmtoffset_minute;
	localtime_r(&t, &tm);
	gmtoffset_hour = 0;
	gmtoffset_minute = 0;
	snprintf(str, 30, "%u-%u%u-%u%uT%u%u:%u%u:%u%u+%u%u:%u%u",
			tm.tm_year + 1900, (tm.tm_mon + 1)/10, (tm.tm_mon + 1)%10,
			tm.tm_mday/10, tm.tm_mday%10, tm.tm_hour/10, tm.tm_hour%10,
			tm.tm_min/10, tm.tm_min%10, tm.tm_sec/10, tm.tm_sec%10,
			0, 0, 0,0);

}

static void delete_notification_info(WsNotificationInfoH notificationInfo) {
	if(notificationInfo) {
		ws_xml_destroy_doc(notificationInfo->EventContent);
		ws_xml_destroy_doc(notificationInfo->headerOpaqueData);
		u_free(notificationInfo->EventAction);
		u_free(notificationInfo);
	}
}
#endif


static WsSubscribeInfo*
search_pull_subs_info(SoapH soap, WsXmlDocH indoc)
{
	WsSubscribeInfo *subsInfo = NULL;
	char *uuid = NULL;
	lnode_t *lnode;
	WsContextH soapCntx = ws_get_soap_context(soap);
	WsXmlNodeH node = ws_xml_get_soap_body(indoc);

	node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_PULL);
	if(node) {
		node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
		uuid = ws_xml_get_node_text(node);
	}
	if(uuid == NULL) return subsInfo;
	pthread_mutex_lock(&soap->lockSubs);
	lnode = list_first(soapCntx->subscriptionMemList);
	while(lnode) {
		subsInfo = (WsSubscribeInfo *)lnode->list_data;
		if(!strcmp(subsInfo->subsId, uuid+5)) break;
		lnode = list_next(soapCntx->subscriptionMemList, lnode);
	}
	pthread_mutex_unlock(&soap->lockSubs);
	if(lnode == NULL) return NULL;
	return subsInfo;
}


static WsXmlDocH
create_enum_info(SoapOpH op,
		 WsContextH epcntx,
              	 WsXmlDocH indoc,
		 WsEnumerateInfo **eInfo)
{
	WsXmlNodeH node = ws_xml_get_soap_body(indoc);
	WsXmlNodeH header = ws_xml_get_soap_header(indoc);
	WsXmlDocH outdoc = NULL;
	WsXmlNodeH enumnode = NULL;
	WsEnumerateInfo *enumInfo;
	WsmanMessage *msg = wsman_get_msg_from_op(op);
	WsmanFaultCodeType fault_code = WSMAN_RC_OK;
	WsmanFaultDetailType fault_detail_code = WSMAN_DETAIL_OK;
	char *uri, *to;

	enumInfo = (WsEnumerateInfo *)u_zalloc(sizeof (WsEnumerateInfo));
	if (enumInfo == NULL) {
		error("No memory");
		fault_code = WSMAN_INTERNAL_ERROR;
		goto DONE;
	}
	enumInfo->encoding = u_strdup(msg->charset);
	enumInfo->maxsize = wsman_get_maxsize_from_op(op);
	if(enumInfo->maxsize == 0) {
		enumnode = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE);
		enumInfo->maxsize = ws_deserialize_uint32(NULL, enumnode,
					     0, XML_NS_ENUMERATION,
					     WSENUM_MAX_CHARACTERS);
	}
	enumInfo->releaseproc = wsman_get_release_endpoint(epcntx, indoc);
	to = ws_xml_get_node_text(
			ws_xml_get_child(header, 0, XML_NS_ADDRESSING, WSA_TO));
	uri =  ws_xml_get_node_text(
			ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_RESOURCE_URI));

	enumInfo->epr_to = u_strdup(to);
	enumInfo->epr_uri = u_strdup(uri);
	node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE);
	node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_EXPIRES);
	if (node == NULL) {
		debug("No wsen:Expires");
		enumInfo->expires = 0;
	} else {
		wsman_set_expiretime(node, &(enumInfo->expires), &fault_code);
		if (fault_code != WSMAN_RC_OK) {
			fault_detail_code = WSMAN_DETAIL_EXPIRATION_TIME;
			goto DONE;
		}
	}

	if (msg->auth_data.username != NULL) {
		enumInfo->auth_data.username =
				u_strdup(msg->auth_data.username);
		enumInfo->auth_data.password =
				u_strdup(msg->auth_data.password);
	} else {
		enumInfo->auth_data.username = NULL;
		enumInfo->auth_data.password = NULL;
	}
	generate_uuid(enumInfo->enumId, EUIDLEN, 1);

DONE:
	if (fault_code != WSMAN_RC_OK) {
		outdoc = wsman_generate_fault(indoc, fault_code, fault_detail_code, NULL);
		u_free(enumInfo);
	} else {
		*eInfo = enumInfo;
	}
	return outdoc;
}

static void
destroy_enuminfo(WsEnumerateInfo * enumInfo)
{
	debug("destroy enuminfo");
	u_free(enumInfo->auth_data.username);
	u_free(enumInfo->auth_data.password);
	u_free(enumInfo->epr_to);
	u_free(enumInfo->epr_uri);
	u_free(enumInfo->encoding);
	if (enumInfo->filter)
		filter_destroy(enumInfo->filter);
	u_free(enumInfo);
}

static int
wsman_verify_enum_info(SoapOpH op,
		       WsEnumerateInfo * enumInfo,
		       WsXmlDocH doc,
		       WsmanStatus * status)
{

	WsmanMessage *msg = wsman_get_msg_from_op(op);
	WsXmlNodeH header = ws_xml_get_soap_header(doc);

	char *to = ws_xml_get_node_text(ws_xml_get_child(header, 0,
			XML_NS_ADDRESSING, WSA_TO));
	char *uri= ws_xml_get_node_text(ws_xml_get_child(header, 0,
			XML_NS_WS_MAN, WSM_RESOURCE_URI));

	if (strcmp(enumInfo->epr_to, to) != 0 ||
			strcmp(enumInfo->epr_uri, uri) != 0 ) {
		status->fault_code = WSA_MESSAGE_INFORMATION_HEADER_REQUIRED;
		status->fault_detail_code = 0;
		debug("verifying enumeration context: ACTUAL  uri: %s, to: %s", uri, to);
		debug("verifying enumeration context: SHOULD uri: %s, to: %s",
				enumInfo->epr_uri, enumInfo->epr_to);
		return 0;
	}

	if (msg->auth_data.username && msg->auth_data.password) {
		if (strcmp(msg->auth_data.username,
				enumInfo->auth_data.username) != 0 &&
				strcmp(msg->auth_data.password,
				enumInfo->auth_data.password) != 0) {
			status->fault_code = WSMAN_ACCESS_DENIED;
			status->fault_detail_code = 0;
			return 0;
		}
	}
	return 1;
}



static int
insert_enum_info(WsContextH cntx,
		WsEnumerateInfo *enumInfo)
{
	struct timeval tv;
	int retVal = 1;

	u_lock(cntx->soap);
	gettimeofday(&tv, NULL);
	enumInfo->timeStamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	if (create_context_entry(cntx->enuminfos, enumInfo->enumId, enumInfo)) {
		retVal = 0;
	}
	u_unlock(cntx->soap);
	return retVal;
}



static WsEnumerateInfo *
get_locked_enuminfo(WsContextH cntx,
                    WsXmlDocH doc,
                    SoapOpH op,
                    char *action,
                    WsmanStatus *status)
{
	hnode_t *hn;
	WsEnumerateInfo *eInfo = NULL;
	char *enumId = NULL;
	WsXmlNodeH node = ws_xml_get_soap_body(doc);

	if (node && (node = ws_xml_get_child(node,
			0, XML_NS_ENUMERATION, action))) {
		node = ws_xml_get_child(node, 0,
			XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
		if (node) {
			enumId = ws_xml_get_node_text(node);
		}
	}
	debug("enum context: %s", enumId);

	if (enumId == NULL) {
		status->fault_code = WSEN_INVALID_ENUMERATION_CONTEXT;
		return NULL;
	}
	u_lock(cntx->soap);
	hn = hash_lookup(cntx->enuminfos, enumId);
	if (hn) {
		eInfo = (WsEnumerateInfo *)hnode_get(hn);
		if (strcmp(eInfo->enumId, enumId)) {
			error("enum context mismatch: %s == %s",
			     eInfo->enumId, enumId);
			status->fault_code = WSMAN_INTERNAL_ERROR;
		} else if (wsman_verify_enum_info(op, eInfo, doc,status)) {
			if (eInfo->flags & WSMAN_ENUMINFO_INWORK_FLAG) {
				status->fault_code = WSMAN_CONCURRENCY;
			} else {
				eInfo->flags |= WSMAN_ENUMINFO_INWORK_FLAG;
			}
		}
	} else {
		status->fault_code = WSEN_INVALID_ENUMERATION_CONTEXT;
	}

	if (status->fault_code != WSMAN_RC_OK) {
		eInfo = NULL;
	}
	u_unlock(cntx->soap);
	return eInfo;
}

static void
unlock_enuminfo(WsContextH cntx, WsEnumerateInfo *enumInfo)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	u_lock(cntx->soap);
	if (!(enumInfo->flags & WSMAN_ENUMINFO_INWORK_FLAG)) {
		error("locked enuminfo unlocked");
		u_unlock(cntx->soap);
		return;
	}
	enumInfo->flags &= ~WSMAN_ENUMINFO_INWORK_FLAG;
	enumInfo->timeStamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	u_unlock(cntx->soap);
}

static void
ws_clear_context_entries(WsContextH hCntx)
{
	hash_t *h;
	if (!hCntx) {
		return;
	}
	h = hCntx->entries;
	hash_free(h);
}

static void
ws_clear_context_enuminfos(WsContextH hCntx)
{
	hash_t *h;
	if (!hCntx) {
		return;
	}
	h = hCntx->enuminfos;
	hash_free(h);
}

callback_t *
make_callback_entry(SoapServiceCallback proc,
		    void *data,
		    list_t * list_to_add)
{
	callback_t *entry = (callback_t *) u_malloc(sizeof(callback_t));
	debug("make new callback entry");
	if (entry) {
		lnode_init(&entry->node, data);
		entry->proc = proc;
		if (list_to_add == NULL) {
			list_to_add = list_create(LISTCOUNT_T_MAX);
		}
		list_append(list_to_add, &entry->node);
	} else {
		return NULL;
	}
	return entry;
}



void
ws_initialize_context(WsContextH cntx, SoapH soap)
{
	cntx->entries = hash_create(HASHCOUNT_T_MAX, NULL, NULL);
	hash_set_allocator(cntx->entries, NULL, free_hentry_func, NULL);

	cntx->enuminfos = hash_create(HASHCOUNT_T_MAX, NULL, NULL);
	cntx->subscriptionMemList = list_create(LISTCOUNT_T_MAX);
	hash_set_allocator(cntx->enuminfos, NULL, free_hentry_func, NULL);
	cntx->owner = 1;
	cntx->soap = soap;
	cntx->serializercntx = ws_serializer_init();
}

WsContextH
ws_create_context(SoapH soap)
{
	WsContextH cntx = (WsContextH) u_zalloc(sizeof (*cntx));
	if (cntx) {
		ws_initialize_context(cntx, soap);
	}
	return cntx;
}

SoapH
ws_soap_initialize()
{
	SoapH soap = (SoapH) u_zalloc(sizeof(*soap));

	if (soap == NULL) {
		error("Could not alloc memory");
		return NULL;
	}
	soap->cntx = ws_create_context(soap);

	soap->inboundFilterList = NULL;
	soap->outboundFilterList = NULL;
	soap->dispatchList = NULL;
	soap->processedMsgIdList = NULL;

	u_init_lock(soap);
	u_init_lock(&soap->lockSubs);
	ws_xml_parser_initialize();

	soap_add_filter(soap, outbound_addressing_filter, NULL, 0);
	soap_add_filter(soap, outbound_control_header_filter, NULL, 0);
	return soap;
}

void
ws_set_context_enumIdleTimeout(WsContextH cntx,
                               unsigned long timeout)
{
	cntx->enumIdleTimeout = timeout;
}



WsContextH
ws_create_runtime(list_t * interfaces)
{
	SoapH soap = ws_soap_initialize();
	WsManDispatcherInfo *dispInfo;
	int size;
	lnode_t *node;
	if (soap == NULL) {
		error("Could not initialize soap");
		return NULL;
	}

	if (interfaces == NULL) {
		return soap->cntx;
	}

	size = calculate_map_count(interfaces);
	dispInfo = (WsManDispatcherInfo *) u_zalloc(size);
	if (dispInfo == NULL) {
		error("Could not allocate memory");
		u_free(soap);
		return NULL;
	}
	debug("Registering %d plugins", (int) list_count(interfaces));
	dispInfo->interfaceCount = list_count(interfaces);
	dispInfo->interfaces = interfaces;
	node = list_first(interfaces);
	while (node != NULL) {
		if (wsman_register_interface(soap->cntx,
				(WsDispatchInterfaceInfo *) node->list_data,
				 dispInfo) != 0) {
			error("Interface registeration failed");
			u_free(dispInfo);
			soap_destroy(soap);
			return NULL;
		}
		node = list_next(interfaces, node);
	}
	ws_register_dispatcher(soap->cntx, wsman_dispatcher, dispInfo);
	return soap->cntx;
}




/**
 * Register Dispatcher Interfaces
 * @param cntx WS-Man Context
 * @param wsInterface Interface
 * @param dispinfo Dispatcher
 */
int
wsman_register_interface(WsContextH cntx,
			 WsDispatchInterfaceInfo * wsInterface,
			 WsManDispatcherInfo * dispInfo)
{
	int i, retVal = 0;
	WsDispatchEndPointInfo *ep = wsInterface->endPoints;

	for (i = 0; ep[i].serviceEndPoint != NULL; i++) {
		if ((retVal = wsman_register_endpoint(cntx, wsInterface,
						  &ep[i], dispInfo)) != 0) {
			break;
		}
	}
	return retVal;
}


/*
 * Register Endpoint
 * @param cntx Context
 * @param wsInterface Interface
 * @param ep Endpoint
 * @param dispInfo Dispatcher information
 */
int
wsman_register_endpoint(WsContextH cntx,
			WsDispatchInterfaceInfo *wsInterface,
			WsDispatchEndPointInfo *ep,
			WsManDispatcherInfo *dispInfo)
{
	SoapDispatchH   disp = NULL;
	unsigned long   flags = SOAP_CUSTOM_DISPATCHER;
	SoapServiceCallback callbackProc = NULL;
	SoapH soap = ws_context_get_runtime(cntx);
	char *action = NULL;

	debug("Registering Endpoint: %s", ep->inAction);
	switch (ep->flags & WS_DISP_TYPE_MASK) {
	case WS_DISP_TYPE_IDENTIFY:
		debug("Registering endpoint for Identify");
		action = ep->inAction;
		callbackProc = wsman_identify_stub;
		break;
	case WS_DISP_TYPE_ENUMERATE:
		debug("Registering endpoint for Enumerate");
		action = ep->inAction;
		callbackProc = wsenum_enumerate_stub;
		break;
	case WS_DISP_TYPE_RELEASE:
		debug("Registering endpoint for Release");
		action = ep->inAction;
		callbackProc = wsenum_release_stub;
		break;
	case WS_DISP_TYPE_DELETE:
		debug("Registering endpoint for Delete");
		action = ep->inAction;
		callbackProc = ws_transfer_delete_stub;
		break;
	case WS_DISP_TYPE_PULL:
		debug("Registering endpoint for Pull");
		action = ep->inAction;
		callbackProc = wsenum_pull_stub;
		break;
	case WS_DISP_TYPE_DIRECT_PULL:
		debug("Registering endpoint for direct Pull");
		action = ep->inAction;
		callbackProc = wsenum_pull_direct_stub;
		break;
	case WS_DISP_TYPE_GET:
		debug("Registering endpoint for Get");
		action = ep->inAction;
		callbackProc = ws_transfer_get_stub;
		break;
	case WS_DISP_TYPE_DIRECT_GET:
		debug("Registering endpoint for direct Get");
		action = ep->inAction;
		callbackProc = (SoapServiceCallback) ep->serviceEndPoint;
		break;
	case WS_DISP_TYPE_DIRECT_DELETE:
		debug("Registering endpoint for Delete");
		action = ep->inAction;
		callbackProc = (SoapServiceCallback) ep->serviceEndPoint;
		break;
	case WS_DISP_TYPE_DIRECT_PUT:
		debug("Registering endpoint for direct Put");
		action = ep->inAction;
		callbackProc = (SoapServiceCallback) ep->serviceEndPoint;
		break;
	case WS_DISP_TYPE_DIRECT_CREATE:
		debug("Registering endpoint for direct Create");
		action = ep->inAction;
		callbackProc = (SoapServiceCallback) ep->serviceEndPoint;
		break;
	case WS_DISP_TYPE_PUT:
		debug("Registering endpoint for Put");
		action = ep->inAction;
		callbackProc = ws_transfer_put_stub;
		break;
#ifdef ENABLE_EVENTING_SUPPORT
	case WS_DISP_TYPE_SUBSCRIBE:
		debug("Registering endpoint for Subscribe");
		action = ep->inAction;
		callbackProc = wse_subscribe_stub;
		break;
	case WS_DISP_TYPE_UNSUBSCRIBE:
		debug("Registering endpoint for Unsubscribe");
		action = ep->inAction;
		callbackProc = wse_unsubscribe_stub;
		break;
	case WS_DISP_TYPE_RENEW:
		action = ep->inAction;
		callbackProc = wse_renew_stub;
		break;
#endif
	case WS_DISP_TYPE_RAW_DOC:
		action = ep->inAction;
		callbackProc = (SoapServiceCallback) ep->serviceEndPoint;
		break;
	case WS_DISP_TYPE_CUSTOM_METHOD:
		debug("Registering endpoint for custom method");
		action = ep->inAction;
		callbackProc = (SoapServiceCallback) ep->serviceEndPoint;
		break;

	case WS_DISP_TYPE_PRIVATE:
		debug("Registering endpoint for private EndPoint");
		action = ep->inAction;
		callbackProc = (SoapServiceCallback) ep->serviceEndPoint;
		break;

	default:
		debug("unknown dispatch type %lu",
			ep->flags & WS_DISP_TYPE_MASK);
		break;
	}

	if (callbackProc != NULL &&
			(disp = wsman_dispatch_create(soap, action, NULL, NULL, callbackProc, ep, flags))) {
		dispInfo->map[dispInfo->mapCount].ep = ep;
		dispInfo->map[dispInfo->mapCount].disp = disp;
		dispInfo->mapCount++;
		wsman_dispatch_start(disp);
	}
	if (action && action != ep->inAction) {
		u_free(action);
	}
	return (disp == NULL);
}

/*  ENDPOINTS STUBS */

int
wsman_identify_stub(SoapOpH op,
		    void *appData,
			void *opaqueData)
{
	void           *data;
	WsXmlDocH       doc = NULL;
	WsContextH      cntx;
	WsDispatchEndPointInfo *info;
	XmlSerializerInfo *typeInfo;
	WsmanStatus    *status;
	SoapH           soap;
	WsEndPointGet   endPoint;

	status = u_zalloc(sizeof(WsmanStatus *));
	soap = soap_get_op_soap(op);
	cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
	info = (WsDispatchEndPointInfo *) appData;
	typeInfo = info->serializationInfo;
	endPoint = (WsEndPointGet) info->serviceEndPoint;
	debug("Identify called");

	if ((data = endPoint(cntx, status, opaqueData)) == NULL) {
		error("Identify Fault");
		doc = wsman_generate_fault(soap_get_op_doc(op, 1), WSMAN_INTERNAL_ERROR, 0, NULL);
	} else {
		doc = wsman_create_response_envelope(soap_get_op_doc(op, 1), NULL);
		ws_serialize(cntx->serializercntx, ws_xml_get_soap_body(doc), data, typeInfo,
			WSMID_IDENTIFY_RESPONSE, (char *) info->data, NULL, 1);
		ws_serializer_free_mem(cntx->serializercntx, data, typeInfo);
	}

	if (doc) {
		soap_set_op_doc(op, doc, 0);
	} else {
		error("Response doc invalid");
	}

	ws_destroy_context(cntx);
	u_free(status);

	return 0;
}




int
ws_transfer_put_stub(SoapOpH op,
		     void *appData,
			void *opaqueData)
{
	int             retVal = 0;
	WsXmlDocH       doc = NULL;
	void           *outData = NULL;
	WsmanStatus     status;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH   cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
	WsDispatchEndPointInfo *info = (WsDispatchEndPointInfo *) appData;
	XmlSerializerInfo *typeInfo = info->serializationInfo;
	WsEndPointPut   endPoint = (WsEndPointPut) info->serviceEndPoint;

	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	WsXmlNodeH      _body = ws_xml_get_soap_body(_doc);
	WsXmlNodeH      _r = ws_xml_get_child(_body, 0, NULL, NULL);

	void  *data = ws_deserialize(cntx->serializercntx, _body, typeInfo,
					ws_xml_get_node_local_name(_r),
			    	(char *) info->data, NULL, 0, 0);

	if ((retVal = endPoint(cntx, data, &outData, &status, opaqueData))) {
		doc = wsman_generate_fault( _doc, status.fault_code,
					   status.fault_detail_code, NULL);
	} else {
		doc = wsman_create_response_envelope(_doc, NULL);
		if (outData) {
			ws_serialize(cntx->serializercntx, ws_xml_get_soap_body(doc), outData,
				     typeInfo, TRANSFER_PUT_RESP,
                     (char *) info->data, NULL, 1);
			ws_serializer_free_mem(cntx->serializercntx, outData, typeInfo);
		}
	}

	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	ws_serializer_free_all(cntx->serializercntx);
	ws_serializer_cleanup(cntx->serializercntx);
	return retVal;
}




int
ws_transfer_delete_stub(SoapOpH op,
		     void *appData,
			void *opaqueData)
{
	WsmanStatus     status;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH      cntx = ws_create_ep_context(soap,
					soap_get_op_doc(op, 1));

	WsDispatchEndPointInfo *info = (WsDispatchEndPointInfo *) appData;
	WsEndPointGet   endPoint = (WsEndPointGet) info->serviceEndPoint;

	void           *data;
	WsXmlDocH       doc = NULL;

	wsman_status_init(&status);
	if ((data = endPoint(cntx, &status, opaqueData)) == NULL) {
		warning("Transfer Delete fault");
		doc = wsman_generate_fault(soap_get_op_doc(op, 1),
					 WSMAN_INVALID_SELECTORS, 0, NULL);
	} else {
		debug("Creating Response doc");
		doc = wsman_create_response_envelope(soap_get_op_doc(op, 1), NULL);
	}

	if (doc) {
		soap_set_op_doc(op, doc, 0);
	} else {
		error("Response doc invalid");
	}
	ws_destroy_context(cntx);
	return 0;
}




int
ws_transfer_get_stub(SoapOpH op,
		     void *appData,
			void *opaqueData)
{
	WsmanStatus     status;


	SoapH       soap = soap_get_op_soap(op);
	WsContextH  cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));

	WsDispatchEndPointInfo *info = (WsDispatchEndPointInfo *) appData;
	XmlSerializerInfo *typeInfo = info->serializationInfo;
	WsEndPointGet   endPoint = (WsEndPointGet) info->serviceEndPoint;

	void           *data;
	WsXmlDocH       doc = NULL;

	wsman_status_init(&status);
	if ((data = endPoint(cntx, &status, opaqueData)) == NULL) {
		warning("Transfer Get fault");
		doc = wsman_generate_fault( soap_get_op_doc(op, 1),
					 WSMAN_INVALID_SELECTORS, 0, NULL);
	} else {
		debug("Creating Response doc");
		doc = wsman_create_response_envelope(soap_get_op_doc(op, 1), NULL);

		ws_serialize(cntx->serializercntx, ws_xml_get_soap_body(doc), data, typeInfo,
			 TRANSFER_GET_RESP, (char *) info->data, NULL, 1);
		ws_serializer_free_mem(cntx->serializercntx, data, typeInfo);
	}

	if (doc) {
		debug("Setting operation document");
		soap_set_op_doc(op, doc, 0);
	} else {
		warning("Response doc invalid");
	}

	ws_destroy_context(cntx);
	return 0;
}



WsmanMessage *wsman_get_msg_from_op(SoapOpH op)
{
	op_t *_op = (op_t *)op;
	WsmanMessage *msg = (WsmanMessage *)_op->data;
	return msg;
}

unsigned long wsman_get_maxsize_from_op(SoapOpH op)
{
	op_t *_op = (op_t *)op;
	return _op->maxsize;
}

/**
 * Enumeration Stub for processing enumeration requests
 * @param op SOAP pperation handler
 * @param appData Application data
 * @return status
 */
int
wsenum_enumerate_stub(SoapOpH op,
		      void *appData,
			void *opaqueData)
{
	WsXmlDocH       doc = NULL;
	int             retVal = 0;
	WsEnumerateInfo *enumInfo = NULL;
	WsmanStatus     status;
	WsXmlNodeH      resp_node, body;
	WsContextH      soapCntx;
	SoapH           soap = soap_get_op_soap(op);

	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;
	WsEndPointEnumerate endPoint =
			(WsEndPointEnumerate)ep->serviceEndPoint;

	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	WsContextH      epcntx;

	epcntx = ws_create_ep_context(soap, _doc);
	wsman_status_init(&status);
	doc = create_enum_info(op, epcntx, _doc, &enumInfo);
	if (doc != NULL) {
		/* wrong enum elements met. Fault message generated */
		goto DONE;
	}

	if (endPoint && (retVal = endPoint(epcntx, enumInfo, &status, opaqueData))) {
                debug("enumeration fault");
		doc = wsman_generate_fault( _doc, status.fault_code,
				status.fault_detail_code, NULL);
		destroy_enuminfo(enumInfo);
		goto DONE;
	}
	if (enumInfo->pullResultPtr) {
		doc = enumInfo->pullResultPtr;
		enumInfo->index++;
	} else {
		doc = wsman_create_response_envelope( _doc, NULL);
	}

	if (!doc)
		goto DONE;

	wsman_set_estimated_total(_doc, doc, enumInfo);
	body = ws_xml_get_soap_body(doc);

	if (enumInfo->pullResultPtr == NULL) {
		resp_node = ws_xml_add_child(body, XML_NS_ENUMERATION,
					     WSENUM_ENUMERATE_RESP, NULL);
	} else {
		resp_node = ws_xml_get_child(body, 0,
				 XML_NS_ENUMERATION, WSENUM_ENUMERATE_RESP);
	}

	soapCntx = ws_get_soap_context(soap);
	if (( enumInfo->flags & WSMAN_ENUMINFO_OPT ) == WSMAN_ENUMINFO_OPT  &&
		(enumInfo->totalItems == 0 || enumInfo->index == enumInfo->totalItems)) {
		ws_serialize_str(epcntx->serializercntx, resp_node, NULL,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
		ws_serialize_str(epcntx->serializercntx, resp_node,
			       NULL, XML_NS_WS_MAN, WSENUM_END_OF_SEQUENCE, 0);
		destroy_enuminfo(enumInfo);
	} else {
		ws_serialize_str(epcntx->serializercntx, resp_node, enumInfo->enumId,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
		insert_enum_info(soapCntx, enumInfo);
	}

DONE:
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	ws_destroy_context(epcntx);
	return retVal;
}



int
wsenum_release_stub(SoapOpH op,
		    void *appData,
			void *opaqueData)
{
	int             retVal = 0;
	WsXmlDocH       doc = NULL;
	WsmanStatus     status;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH      soapCntx = ws_get_soap_context(soap);
	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;
	WsEndPointRelease endPoint = (WsEndPointRelease) ep->serviceEndPoint;
	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	WsEnumerateInfo *enumInfo;

	wsman_status_init(&status);
	enumInfo = get_locked_enuminfo(soapCntx, _doc,
		op, WSENUM_RELEASE, &status);

	if (enumInfo == NULL) {
		doc = wsman_generate_fault( _doc,
			status.fault_code, status.fault_detail_code, NULL);

	} else {
		if (endPoint && (retVal = endPoint(soapCntx,
						enumInfo, &status, opaqueData))) {
			error("endPoint error");
			doc = wsman_generate_fault( _doc,
				WSMAN_INTERNAL_ERROR,
				OWSMAN_DETAIL_ENDPOINT_ERROR, NULL);
			unlock_enuminfo(soapCntx, enumInfo);
		} else {
			doc = wsman_create_response_envelope( _doc, NULL);
			debug("Releasing context: %s", enumInfo->enumId);
			remove_locked_enuminfo(soapCntx, enumInfo);
			destroy_enuminfo(enumInfo);
		}
	}
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	return retVal;
}





int
wsenum_pull_stub(SoapOpH op, void *appData,
			void *opaqueData)
{
	WsXmlNodeH      node;
	WsmanStatus     status;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH      soapCntx = ws_get_soap_context(soap);

	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;
	XmlSerializerInfo *typeInfo = ep->serializationInfo;
	WsEndPointPull  endPoint = (WsEndPointPull) ep->serviceEndPoint;

	int retVal = 0, locked = 0;
	WsXmlDocH doc = NULL;
	char *enumId = NULL;

	WsXmlDocH _doc = soap_get_op_doc(op, 1);
	WsEnumerateInfo *enumInfo;

	wsman_status_init(&status);
	enumInfo = get_locked_enuminfo(soapCntx, _doc,
		op, WSENUM_PULL, &status);

	if (enumInfo == NULL) {
		doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
		goto DONE;
	}
	locked = 1;
	if ((retVal = endPoint(ws_create_ep_context(soap, _doc),
						enumInfo, &status, opaqueData))) {
		doc = wsman_generate_fault(_doc, status.fault_code, status.fault_detail_code, NULL);
		goto DONE;
	}
	enumInfo->index++;
	doc = wsman_create_response_envelope( _doc, NULL);

	if (!doc) {
		goto DONE;
	}

	wsman_set_estimated_total(_doc, doc, enumInfo);
	node = ws_xml_add_child(ws_xml_get_soap_body(doc),
				XML_NS_ENUMERATION, WSENUM_PULL_RESP, NULL);

	if (node == NULL) {
		goto DONE;
	}
	if (enumInfo->pullResultPtr) {
		WsXmlNodeH itemsNode = ws_xml_add_child(node,
				    XML_NS_ENUMERATION, WSENUM_ITEMS, NULL);
		ws_serialize(soapCntx->serializercntx, itemsNode, enumInfo->pullResultPtr,
			 typeInfo, ep->respName, (char *) ep->data, NULL, 1);
		if (enumId) {
			ws_serialize_str(soapCntx->serializercntx, node, enumId,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
		}
		ws_serializer_free_mem(soapCntx->serializercntx,
			enumInfo->pullResultPtr, typeInfo);
	} else {
		/*
		ws_serialize_str(soapCntx, node, NULL,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
			    */
		ws_serialize_str(soapCntx->serializercntx,
		    node, NULL, XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE, 0);
		remove_locked_enuminfo(soapCntx, enumInfo);
		locked = 0;
		destroy_enuminfo(enumInfo);
	}

DONE:
	if (locked) {
		unlock_enuminfo(soapCntx, enumInfo);
	}
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	return retVal;
}


int
wsenum_pull_direct_stub(SoapOpH op,
		     void *appData,
			void *opaqueData)
{
	WsmanStatus     status;
	WsXmlDocH       doc = NULL;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH      soapCntx = ws_get_soap_context(soap);
	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;
#ifdef ENABLE_EVENTING_SUPPORT
	WsNotificationInfoH notificationInfo = NULL;
#endif

	WsEndPointPull  endPoint = (WsEndPointPull) ep->serviceEndPoint;
	int             retVal = 0;
	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	int locked = 0;
	WsEnumerateInfo *enumInfo;
	WsSubscribeInfo *subsInfo = NULL;
	wsman_status_init(&status);
	enumInfo = get_locked_enuminfo(soapCntx,
	                               _doc, op, WSENUM_PULL, &status);

	if (enumInfo == NULL) {
		subsInfo = search_pull_subs_info(soap, _doc);
		if(subsInfo == NULL) {
			error("Invalid enumeration context...");
			doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
			goto cleanup;
		}
	}
	if (enumInfo) { //pull things from "enumerate" results
		locked = 1;

		if ((retVal = endPoint(ws_create_ep_context(soap, _doc),
						enumInfo, &status, opaqueData))) {
			doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
//			ws_remove_context_val(soapCntx, cntxName);
			goto cleanup;
		}

		enumInfo->index++;
		if (enumInfo->pullResultPtr) {
			WsXmlNodeH      body;
			WsXmlNodeH      response;
			doc = enumInfo->pullResultPtr;
			wsman_set_estimated_total(_doc, doc, enumInfo);

			body = ws_xml_get_soap_body(doc);
			response = ws_xml_get_child(body, 0,
					XML_NS_ENUMERATION, WSENUM_PULL_RESP);

			if (enumInfo->totalItems == 0 || enumInfo->index == enumInfo->totalItems) {
				/*
				   ws_serialize_str(soapCntx, response, NULL,
				   XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
				   */
				ws_serialize_str(soapCntx->serializercntx, response, NULL,
						XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE, 0);
				remove_locked_enuminfo(soapCntx, enumInfo);
				locked = 0;
				destroy_enuminfo(enumInfo);
			} else  {
				ws_serialize_str(soapCntx->serializercntx, response, enumInfo->enumId,
						XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
			}
		}
	}
#ifdef ENABLE_EVENTING_SUPPORT
	else { //pull things from notifications
		ws_xml_destroy_doc(doc);
		pthread_mutex_lock(&subsInfo->notificationlock);
		int count = soap->eventpoolOpSet->count(subsInfo->subsId);
		int max_elements = 1;
		if(count > 0) {
			doc = ws_xml_create_envelope();
			WsXmlNodeH docnode = ws_xml_get_soap_body(doc);
			WsXmlNodeH docheader = ws_xml_get_soap_header(doc);
			docnode = ws_xml_add_child(docnode, XML_NS_ENUMERATION, WSENUM_PULL_RESP, NULL);
			if(docnode) {
				ws_xml_add_child_format(docnode, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT,
					"uuid:%s", subsInfo->subsId);
			}
			WsXmlDocH notidoc = NULL;
			WsXmlNodeH header = ws_xml_get_soap_header(_doc);
			if (ws_xml_get_child(header, 0,XML_NS_WS_MAN, WSM_REQUEST_TOTAL) != NULL) {
				WsXmlNodeH response_header =ws_xml_get_soap_header(doc);
				response_header = ws_xml_add_child(response_header, XML_NS_WS_MAN,
					WSM_TOTAL_ESTIMATE, NULL);
				if(response_header)
					ws_xml_add_node_attr(response_header, XML_NS_SCHEMA_INSTANCE, XML_SCHEMA_NIL, "true");
			}
			header = ws_xml_get_child(header, 0, XML_NS_ENUMERATION, WSENUM_MAX_ELEMENTS);
			if(header)
				max_elements = atoi(ws_xml_get_node_text(header));
			if(max_elements > 1 && count > 1) {
				docnode = ws_xml_add_child(docnode, XML_NS_ENUMERATION, WSENUM_ITEMS, NULL);
			}
			while(max_elements > 0) {
				if(soap->eventpoolOpSet->remove(subsInfo->subsId, &notificationInfo))
					break;
				ws_xml_add_child(docheader, XML_NS_ADDRESSING, WSA_ACTION, notificationInfo->EventAction);
				notidoc = notificationInfo->EventContent;
				WsXmlNodeH tempnode = ws_xml_get_doc_root(notidoc);
				ws_xml_duplicate_tree(docnode, tempnode);
				delete_notification_info(notificationInfo);
				max_elements--;
			}
		}
		else {
			status.fault_code = WSMAN_TIMED_OUT;
			doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
		}
		pthread_mutex_unlock(&subsInfo->notificationlock);
	}
#endif
cleanup:
	if (locked) {
		unlock_enuminfo(soapCntx, enumInfo);
	}
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	} else {
		error("doc is null");
	}

	return retVal;
}

static list_t *
wsman_get_expired_enuminfos(WsContextH cntx)
{
	list_t *list = NULL;
	hnode_t        *hn;
	hscan_t         hs;
	WsEnumerateInfo *enumInfo;
	struct timeval tv;
	unsigned long mytime;
	unsigned long aeit = cntx->enumIdleTimeout;

	if (aeit == 0) {
		return NULL;
	}
	gettimeofday(&tv, NULL);
	mytime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	u_lock(cntx->soap);
	if (hash_isempty(cntx->enuminfos)) {
		u_unlock(cntx->soap);
		return NULL;
	}
	hash_scan_begin(&hs, cntx->enuminfos);
	while ((hn = hash_scan_next(&hs))) {
		enumInfo = (WsEnumerateInfo *)hnode_get(hn);
		if (enumInfo->flags & WSMAN_ENUMINFO_INWORK_FLAG) {
			debug("Enum in work: %s", enumInfo->enumId);
			continue;
		}
		if ((enumInfo->timeStamp + aeit > mytime) &&
				((enumInfo->expires == 0) ||
				(enumInfo->expires > mytime))) {
			continue;
		}
		if (list == NULL) {
			list = list_create(LISTCOUNT_T_MAX);
		}
		if (list == NULL) {
			u_unlock(cntx->soap);
			error("could not create list");
			return NULL;
		}
		hash_scan_delfree(cntx->enuminfos, hn);
		list_append(list, lnode_create(enumInfo));
		debug("Enum expired list appended: %s", enumInfo->enumId);
	}
	u_unlock(cntx->soap);
	return list;
}

void
wsman_timeouts_manager(WsContextH cntx, void *opaqueData)
{
	list_t *list = wsman_get_expired_enuminfos(cntx);
	lnode_t *node;
	WsEnumerateInfo *enumInfo;
	WsmanStatus status;

	if (list == NULL) {
		return;
	}
	while ((node = list_del_first(list))) {
		enumInfo = (WsEnumerateInfo *)lnode_get(node);
		debug("EnumContext expired : %s", enumInfo->enumId);
		lnode_destroy(node);
		if (enumInfo->releaseproc) {
			if (enumInfo->releaseproc(cntx, enumInfo, &status, opaqueData)) {
				debug("released with failure: %s",
						enumInfo->enumId);
			} else {
				debug("released: %s", enumInfo->enumId);
			}
		} else {
			debug("no release endpoint: %s", enumInfo->enumId);
		}
		destroy_enuminfo(enumInfo);
		if (list_isempty(list)) {
			list_destroy(list);
			break;
		}
	}
	return;
}


#ifdef ENABLE_EVENTING_SUPPORT
static int destination_reachable(char *url)
{
	int valid = 0;
	u_uri_t *uri = NULL;
	if(strstr(url, "http") == NULL)
		return valid;
	if (u_uri_parse((const char *)url, &uri) == 0) {
		valid = 1;
	}
	u_uri_free(uri);
	return valid;
}

WsEventThreadContextH
ws_create_event_context(SoapH soap, WsSubscribeInfo *subsInfo, WsXmlDocH doc)
{
	WsEventThreadContextH eventcntx = u_malloc(sizeof(*eventcntx));
	eventcntx->soap = soap;
	eventcntx->subsInfo = subsInfo;
	eventcntx->outdoc = doc;
	return eventcntx;
}

static void
destroy_subsinfo(WsSubscribeInfo * subsInfo)
{
	if(subsInfo == NULL) return;
	u_free(subsInfo->uri);
	u_free(subsInfo->auth_data.username);
	u_free(subsInfo->auth_data.password);
	u_free(subsInfo->epr_notifyto);
	u_free(subsInfo->locale);
	u_free(subsInfo->soapNs);
	u_free(subsInfo->contentEncoding);
	u_free(subsInfo->cim_namespace);
	u_free(subsInfo->username);
	u_free(subsInfo->password);
	u_free(subsInfo->certificate_thumbprint);
	if (subsInfo->filter) {
		filter_destroy(subsInfo->filter);
	}
	ws_xml_destroy_doc(subsInfo->bookmarkDoc);
	ws_xml_destroy_doc(subsInfo->templateDoc);
	ws_xml_destroy_doc(subsInfo->heartbeatDoc);
	u_free(subsInfo);
}


static void
create_notification_template(WsXmlDocH indoc, WsSubscribeInfo *subsInfo)
{
	WsXmlDocH notificationDoc = ws_xml_create_envelope();
	WsXmlNodeH temp = NULL;
	WsXmlNodeH node = NULL;
	WsXmlNodeH header = NULL;
	header = ws_xml_get_soap_header(notificationDoc);
	ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_TO, subsInfo->epr_notifyto);
	if(subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_EVENTS ||
		subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_PUSHWITHACK) {
		ws_xml_add_child(header, XML_NS_WS_MAN, WSM_ACKREQUESTED, NULL);
	}
	node = ws_xml_get_soap_body(indoc);
	node = ws_xml_get_child(node, 0, XML_NS_EVENTING, WSEVENT_SUBSCRIBE);
	node = ws_xml_get_child(node, 0, XML_NS_EVENTING, WSEVENT_DELIVERY);
	node = ws_xml_get_child(node, 0, XML_NS_EVENTING, WSEVENT_NOTIFY_TO);
	temp = ws_xml_get_child(node, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PROPERTIES);
	if(temp == NULL)
		node = ws_xml_get_child(node, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);
	if(node ) {
		ws_xml_duplicate_children(header, node);
	}
	subsInfo->templateDoc = ws_xml_duplicate_doc(notificationDoc);
	subsInfo->heartbeatDoc = ws_xml_duplicate_doc( notificationDoc);
	temp = ws_xml_get_soap_header(subsInfo->heartbeatDoc);
	temp = ws_xml_add_child(temp, XML_NS_ADDRESSING, WSA_ACTION, WSMAN_ACTION_HEARTBEAT);
	ws_xml_add_node_attr(temp, XML_NS_XML_SCHEMA, SOAP_MUST_UNDERSTAND, "true");
	ws_xml_destroy_doc(notificationDoc);
}


static WsXmlDocH
create_subs_info(SoapOpH op,
		 		WsContextH epcntx,
              	WsXmlDocH indoc,
              	WsSubscribeInfo**sInfo)
{
	WsXmlNodeH  node = ws_xml_get_soap_body(indoc);
	WsXmlNodeH	subNode = ws_xml_get_child(node, 0, XML_NS_EVENTING, WSEVENT_SUBSCRIBE);
	WsXmlNodeH	temp;
	WsXmlDocH outdoc = NULL;
	WsSubscribeInfo *subsInfo;
	WsXmlAttrH attr = NULL;
	op_t *_op = (op_t *) op;
	WsmanMessage   *msg = (WsmanMessage *) _op->data;
	WsmanFaultCodeType fault_code = WSMAN_RC_OK;
	WsmanFaultDetailType fault_detail_code = WSMAN_DETAIL_OK;
	char *str = NULL;
	time_t timeout;
	int r;
	char *soapNs = NULL, *ntext = NULL;


	*sInfo = NULL;
	subsInfo = (WsSubscribeInfo *)u_zalloc(sizeof (WsSubscribeInfo));
	if (subsInfo == NULL) {
		error("No memory");
		fault_code = WSMAN_INTERNAL_ERROR;
		goto DONE;
	}
	if((r = pthread_mutex_init(&subsInfo->notificationlock, NULL)) != 0) {
		fault_code = WSMAN_INTERNAL_ERROR;
		goto DONE;
	}
	subsInfo->uri = u_strdup(wsman_get_resource_uri(epcntx, indoc));
	if(!subNode) {
		message("No subsribe body");
		fault_code = WSE_INVALID_MESSAGE;
		goto DONE;
	}
	soapNs = ws_xml_get_node_name_ns(ws_xml_get_doc_root(indoc));
	subsInfo->soapNs = u_strdup(soapNs);
	node = ws_xml_get_child(subNode, 0, XML_NS_WS_MAN, WSM_SENDBOOKMARKS);
	if(node) {
		subsInfo->bookmarksFlag = 1;
	}
	node = ws_xml_get_child(subNode, 0, XML_NS_WS_MAN, WSM_BOOKMARK);
	if(node) {
		if(ws_xml_get_node_text(node) &&
			!strcmp(ws_xml_get_node_text(node), WSM_DEFAULTBOOKMARK)){
			subsInfo->flags |= WSMAN_SUBSCRIBEINFO_BOOKMARK_DEFAULT;
		}
		else {
			subsInfo->bookmarkDoc = ws_xml_create_doc(XML_NS_WS_MAN, WSM_BOOKMARK);
			temp = ws_xml_get_doc_root(subsInfo->bookmarkDoc);
			ws_xml_duplicate_children(temp, node);
		}
	}
	node = ws_xml_get_child(subNode, 0, XML_NS_EVENTING, WSEVENT_EXPIRES);
	if (node == NULL) {
		debug("No wsen:Expires");
		subsInfo->expires = 0;
	}
	else {
		wsman_set_expiretime(node, &subsInfo->expires, &fault_code);
		if (fault_code != WSMAN_RC_OK) {
			debug("Invalid expiration time!");
			goto DONE;
		}
		if(time_expired(subsInfo->expires)) {
			fault_code = WSE_INVALID_EXPIRATION_TIME;
			debug("Invalid expiration time!");
			goto DONE;
		}
	}
	node = ws_xml_get_child(subNode, 0, XML_NS_EVENTING, WSEVENT_DELIVERY);
	attr = ws_xml_find_node_attr(node, NULL,WSEVENT_DELIVERY_MODE);
	if(attr) {
		str = ws_xml_get_attr_value(attr);
		if (!strcasecmp(str, WSEVENT_DELIVERY_MODE_PUSH)) {
			subsInfo->deliveryMode = WS_EVENT_DELIVERY_MODE_PUSH;
		}
		else if (!strcasecmp(str, WSEVENT_DELIVERY_MODE_PUSHWITHACK)) {
			subsInfo->deliveryMode = WS_EVENT_DELIVERY_MODE_PUSHWITHACK;
		}
		else if (!strcasecmp(str, WSEVENT_DELIVERY_MODE_EVENTS))  {
			subsInfo->deliveryMode = WS_EVENT_DELIVERY_MODE_EVENTS;
		}
		else {
			subsInfo->deliveryMode = WS_EVENT_DELIVERY_MODE_PULL;
		}
	}
	else {
		//"push" is the default delivery mode
		subsInfo->deliveryMode = WS_EVENT_DELIVERY_MODE_PUSH;
	}
	temp = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_CONTENTCODING);
	if(temp){
		str = ws_xml_get_node_text(temp);
		subsInfo->contentEncoding = u_strdup(str);
	}
	temp = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_LOCALE);
	if(temp) {
		attr = ws_xml_find_node_attr(temp, XML_NS_WS_MAN, WSM_LOCALE);
		if(attr)
			subsInfo->locale = u_strdup(ws_xml_get_attr_value(attr));
	}
	temp = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_HEARTBEATS);
	if(temp) {
		str = ws_xml_get_node_text(temp);
		debug("[heartbeat interval = %s]",str);
		if(str[0]=='P') {
			//  xml duration
			if (ws_deserialize_duration(str, &timeout)) {
				fault_code = WSEN_INVALID_EXPIRATION_TIME;
				goto DONE;
			}
			debug("timeout = %d", timeout);
			subsInfo->heartbeatInterval = timeout * 1000;
			subsInfo->heartbeatCountdown = subsInfo->heartbeatInterval;
		}
	}
	if(subsInfo->deliveryMode != WS_EVENT_DELIVERY_MODE_PULL) {
		temp = ws_xml_get_child(node, 0, XML_NS_EVENTING, WSEVENT_NOTIFY_TO);
		if(temp == NULL) {
			message("No notification destination");
			fault_code = WSE_INVALID_MESSAGE;
			goto DONE;
		}
		str = ws_xml_get_node_text(ws_xml_get_child(temp, 0, XML_NS_ADDRESSING, WSA_ADDRESS));
		debug("event sink: %s", str);
		if(str && strcmp(str, "")) {
			subsInfo->epr_notifyto = u_strdup(str);
			if(destination_reachable(str) == 0) {
				fault_code = WSMAN_EVENT_DELIVER_TO_UNUSABLE;
				goto DONE;
			}
		}
		else {
			fault_code = WSE_INVALID_MESSAGE;
			goto DONE;
		}
		temp = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_AUTH);
		if(temp) {
			attr = ws_xml_find_node_attr(temp, NULL, WSM_PROFILE);
			if(attr) {
				str = ws_xml_get_attr_value(attr);
				if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTP_BASIC))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTP_BASIC_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTP_DIGEST))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTP_DIGEST_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTPS_BASIC))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTPS_BASIC_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTPS_DIGEST))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTPS_DIGEST_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_BASIC))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_BASIC_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_DIGEST))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_DIGEST_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTPS_SPNEGO_KERBEROS))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTPS_SPNEGO_KERBEROS_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_SPNEGO_KERBEROS))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_SPNEGO_KERBEROS_TYPE;
				else if(!strcasecmp(str, WSMAN_SECURITY_PROFILE_HTTP_SPNEGO_KERBEROS))
					subsInfo->deliveryAuthType = WSMAN_SECURITY_PROFILE_HTTP_SPNEGO_KERBEROS_TYPE;
				else {
					fault_code = WSMAN_INVALID_OPTIONS;
					fault_detail_code = WSMAN_DETAIL_AUTHERIZATION_MODE;
					goto DONE;
				}
				debug("auth profile type = %d", subsInfo->deliveryAuthType);

			}
		}
	}
	if(wsman_parse_credentials(indoc, subsInfo, &fault_code, &fault_detail_code)) {
		goto DONE;
	}
	if(wsman_parse_event_request(indoc, subsInfo, &fault_code, &fault_detail_code)) {
		goto DONE;
	}
	if (msg->auth_data.username != NULL) {
		subsInfo->auth_data.username =
				u_strdup(msg->auth_data.username);
		subsInfo->auth_data.password =
				u_strdup(msg->auth_data.password);
	} else {
		subsInfo->auth_data.username = NULL;
		subsInfo->auth_data.password = NULL;
	}
	temp = ws_xml_get_soap_header(indoc);
	temp = ws_xml_get_child(temp, 0, XML_NS_OPENWSMAN, "FormerUID");
	ntext = ws_xml_get_node_text(temp);
	if(temp && ntext) { //it is a request from the saved reqeust. So we recover the former UUID
		strncpy(subsInfo->subsId, ntext, EUIDLEN);
		debug("Recover to uuid:%s",subsInfo->subsId);
	}
	else
		generate_uuid(subsInfo->subsId, EUIDLEN, 1);
	if(subsInfo->deliveryMode != WS_EVENT_DELIVERY_MODE_PULL)
		create_notification_template(indoc, subsInfo);
DONE:
	if (fault_code != WSMAN_RC_OK) {
		outdoc = wsman_generate_fault(indoc, fault_code, fault_detail_code, NULL);
		destroy_subsinfo(subsInfo);
	} else {
		*sInfo = subsInfo;
	}
	return outdoc;
}


/**
 * Subscribe Stub for processing subscription requests
 * @param op SOAP pperation handler
 * @param appData Application data
 * @return status
 */
int
wse_subscribe_stub(SoapOpH op, void *appData, void *opaqueData)
{
	WsXmlDocH       doc = NULL;
	int             retVal = 0;
	WsSubscribeInfo *subsInfo = NULL;
	WsmanStatus     status;
	WsXmlNodeH      inNode, body, header, temp;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH soapCntx = ws_get_soap_context(soap);
	int i;
	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;
	WsEndPointSubscribe endPoint =
			(WsEndPointSubscribe)ep->serviceEndPoint;

	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	WsContextH      epcntx;
	char *buf = NULL;
	char *expiresstr = NULL;
	int len;
	epcntx = ws_create_ep_context(soap, _doc);
	wsman_status_init(&status);
	doc = create_subs_info(op, epcntx, _doc, &subsInfo);
	if (doc != NULL) {
		goto DONE;
	}
	if (endPoint && (retVal = endPoint(epcntx, subsInfo, &status, opaqueData))) {
                debug("Subscribe fault");
		doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
		destroy_subsinfo(subsInfo);
		goto DONE;
	}
	doc = wsman_create_response_envelope(_doc, NULL);
	if (!doc)
		goto DONE;

	char str[30];
	wsman_expiretime2xmldatetime(subsInfo->expires, str);
	if(soap->subscriptionOpSet) {
		temp = ws_xml_get_child(ws_xml_get_soap_body(_doc), 0, XML_NS_EVENTING, WSEVENT_SUBSCRIBE);
		temp = ws_xml_get_child(temp, 0, XML_NS_EVENTING, WSEVENT_EXPIRES);
		if(temp) {
			expiresstr = strdup(ws_xml_get_node_text(temp));
			ws_xml_set_node_text(temp, str);
		}
		temp = ws_xml_get_soap_header(_doc);
		inNode = ws_xml_get_child(temp, 0, XML_NS_OPENWSMAN, "FormerUID");
		if(inNode == NULL)
			ws_xml_add_child(temp, XML_NS_OPENWSMAN, "FormerUID", subsInfo->subsId);
		ws_xml_dump_memory_enc(_doc, &buf, &len, "UTF-8");
		if(buf) {
			soap->subscriptionOpSet->save_subscritption(soap->uri_subsRepository, subsInfo->subsId, (unsigned char*)buf);
			u_free(buf);
		}
	}
	lnode_t * sinfo = lnode_create(subsInfo);
	pthread_mutex_lock(&soap->lockSubs);
	list_append(soapCntx->subscriptionMemList, sinfo);
	pthread_mutex_unlock(&soap->lockSubs);
	debug("subscription uuid:%s kept in the memory", subsInfo->subsId);
	header = ws_xml_get_soap_header(doc);
	inNode = ws_xml_get_soap_header(_doc);
	inNode = ws_xml_get_child(inNode, 0, XML_NS_ADDRESSING, WSA_REPLY_TO);
	inNode = ws_xml_get_child(inNode, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PROPERTIES);
	if(inNode == NULL)
		inNode = ws_xml_get_child(inNode, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);
	if(inNode) {
		for (i = 0;
		     (temp =
		      ws_xml_get_child(inNode, i, NULL, NULL)) != NULL;
		     i++) {
			ws_xml_duplicate_tree(header, temp);
		}
	}
	body = ws_xml_get_soap_body(doc);
	inNode = ws_xml_add_child(body, XML_NS_EVENTING, WSEVENT_SUBSCRIBE_RESP, NULL);
	temp = ws_xml_add_child(inNode, XML_NS_EVENTING, WSEVENT_SUBSCRIPTION_MANAGER, NULL);
	if(subsInfo->expires)
		ws_xml_add_child(inNode, XML_NS_EVENTING, WSEVENT_EXPIRES, expiresstr);
	if(subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_PULL)
		ws_xml_add_child_format(inNode, XML_NS_ENUMERATION,
		WSENUM_ENUMERATION_CONTEXT, "uuid:%s", subsInfo->subsId);
	inNode = temp;
	if(inNode){
		temp = ws_xml_get_soap_header(_doc);
		temp = ws_xml_get_child(temp, 0, XML_NS_ADDRESSING, WSA_TO);
		ws_xml_add_child(inNode,XML_NS_ADDRESSING,WSA_ADDRESS,ws_xml_get_node_text(temp));
	}
	temp = ws_xml_add_child(inNode, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS, NULL);
	if(temp)
		ws_xml_add_child_format(temp, XML_NS_EVENTING, WSEVENT_IDENTIFIER, "uuid:%s", subsInfo->subsId);
DONE:
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	u_free(expiresstr);
	ws_serializer_free_all(epcntx->serializercntx);
	ws_destroy_context(epcntx);
	return retVal;
}




/**
 * Unsubscribe Stub for processing unsubscription requests
 * @param op SOAP pperation handler
 * @param appData Application data
 * @return status
 */
int
wse_unsubscribe_stub(SoapOpH op, void *appData, void *opaqueData)
{
	WsXmlDocH       doc = NULL;
	int             retVal = 0;
	WsSubscribeInfo *subsInfo = NULL;
	WsmanStatus     status;
	WsXmlNodeH      inNode;
	WsXmlNodeH      body;
	WsXmlNodeH      header;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH soapCntx = ws_get_soap_context(soap);
	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;
	WsEndPointSubscribe endPoint =
			(WsEndPointSubscribe)ep->serviceEndPoint;

	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	WsContextH      epcntx;

	epcntx = ws_create_ep_context(soap, _doc);
	wsman_status_init(&status);
	body = ws_xml_get_soap_body(_doc);
	header = ws_xml_get_soap_header(_doc);
	inNode = ws_xml_get_child(header, 0, XML_NS_EVENTING, WSEVENT_IDENTIFIER);
	if(inNode == NULL) {
		status.fault_code = WSE_INVALID_MESSAGE;
		status.fault_detail_code = WSMAN_DETAIL_INVALID_VALUE;
		goto DONE;
	}
	char *uuid = ws_xml_get_node_text(inNode);
	lnode_t *t = NULL;
	pthread_mutex_lock(&soap->lockSubs);
	if(!list_isempty(soapCntx->subscriptionMemList)) {
		t = list_first(soapCntx->subscriptionMemList);
		subsInfo = (WsSubscribeInfo *)t->list_data;
		while(t && strcasecmp(subsInfo->subsId, uuid+5)) {
			t = list_next(soapCntx->subscriptionMemList, t);
			if(t)
				subsInfo = (WsSubscribeInfo *)t->list_data;
		}
	}
	if(t == NULL) {
		status.fault_code = WSMAN_INVALID_PARAMETER;
		status.fault_detail_code = WSMAN_DETAIL_INVALID_VALUE;
		doc = wsman_generate_fault( _doc,
		 	status.fault_code, status.fault_detail_code, NULL);
		pthread_mutex_unlock(&soap->lockSubs);
		goto DONE;
	}
	pthread_mutex_unlock(&soap->lockSubs);
	if (endPoint && (retVal = endPoint(epcntx, subsInfo, &status, opaqueData))) {
               debug("UnSubscribe fault");
		doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
		goto DONE;
	}
	pthread_mutex_lock(&subsInfo->notificationlock);
	subsInfo->flags |= WSMAN_SUBSCRIBEINFO_UNSUBSCRIBE;
	pthread_mutex_unlock(&subsInfo->notificationlock);
	debug("subscription %s unsubscribed", uuid);
	doc = wsman_create_response_envelope( _doc, NULL);
	if (!doc)
		goto DONE;
DONE:
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	ws_serializer_free_all(epcntx->serializercntx);
	ws_destroy_context(epcntx);
	return retVal;
}


/**
 * Renew Stub for processing renew requests
 * @param op SOAP pperation handler
 * @param appData Application data
 * @return status
 */
int
wse_renew_stub(SoapOpH op, void *appData, void *opaqueData)
{
	WsXmlDocH       doc = NULL;
	int             retVal = 0;
	WsSubscribeInfo *subsInfo;
	WsmanStatus     status;
	WsXmlNodeH      inNode;
	WsXmlNodeH      body;
	WsXmlNodeH      header;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH soapCntx = ws_get_soap_context(soap);
	char * expirestr = NULL;

	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;
	WsEndPointSubscribe endPoint =
			(WsEndPointSubscribe)ep->serviceEndPoint;

	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	WsContextH      epcntx;
	epcntx = ws_create_ep_context(soap, _doc);
	wsman_status_init(&status);
	body = ws_xml_get_soap_body(_doc);
	header = ws_xml_get_soap_header(_doc);
	inNode = ws_xml_get_child(header, 0, XML_NS_EVENTING, WSEVENT_IDENTIFIER);
	char *uuid = ws_xml_get_node_text(inNode);
	if(uuid == NULL) {
		status.fault_code = WSE_INVALID_MESSAGE;
		status.fault_detail_code = WSMAN_DETAIL_MISSING_VALUES;
		doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
		goto DONE;
	}
	pthread_mutex_lock(&soap->lockSubs);
	lnode_t *t = NULL;
	if(!list_isempty(soapCntx->subscriptionMemList)) {
		t = list_first(soapCntx->subscriptionMemList);
		subsInfo = (WsSubscribeInfo *)t->list_data;
		while(t && strcasecmp(subsInfo->subsId, uuid+5)) {
			t = list_next(soapCntx->subscriptionMemList, t);
			if(t)
				subsInfo = (WsSubscribeInfo *)t->list_data;
		}
	}
	if(t == NULL) {
		status.fault_code = WSE_UNABLE_TO_RENEW;
		doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
		pthread_mutex_unlock(&soap->lockSubs);
		goto DONE;
	}
	pthread_mutex_unlock(&soap->lockSubs);
	inNode = ws_xml_get_child(body, 0, XML_NS_EVENTING, WSEVENT_RENEW);
	inNode = ws_xml_get_child(inNode, 0, XML_NS_EVENTING ,WSEVENT_EXPIRES);
	pthread_mutex_lock(&subsInfo->notificationlock);
	wsman_set_expiretime(inNode, &subsInfo->expires, &status.fault_code);
	expirestr = ws_xml_get_node_text(inNode);
	pthread_mutex_unlock(&subsInfo->notificationlock);
	if (status.fault_code != WSMAN_RC_OK) {
		status.fault_detail_code = WSMAN_DETAIL_EXPIRATION_TIME;
		pthread_mutex_unlock(&subsInfo->notificationlock);
		goto DONE;
	}
	char str[30];
	wsman_expiretime2xmldatetime(subsInfo->expires, str);
	if(soap->subscriptionOpSet) {
		soap->subscriptionOpSet->update_subscription(soap->uri_subsRepository, uuid+5,
			str);
		debug("subscription %s updated!", uuid);
	}

	if (endPoint && (retVal = endPoint(epcntx, subsInfo, &status, opaqueData))) {
                debug("renew fault in plug-in");
		doc = wsman_generate_fault( _doc, status.fault_code, status.fault_detail_code, NULL);
		pthread_mutex_unlock(&subsInfo->notificationlock);
		goto DONE;
	}
	doc = wsman_create_response_envelope( _doc, NULL);
	if (!doc)
		goto DONE;
	body = ws_xml_get_soap_body(doc);
	body = ws_xml_add_child(body, XML_NS_EVENTING, WSEVENT_RENEW_RESP, NULL);
	ws_xml_add_child(body, XML_NS_EVENTING, WSEVENT_EXPIRES, expirestr);
DONE:
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	ws_serializer_free_all(epcntx->serializercntx);
	ws_destroy_context(epcntx);
	return retVal;
}


void
wsman_heartbeat_generator(WsContextH cntx, void *opaqueData)
{
	SoapH soap = cntx->soap;
	WsSubscribeInfo *subsInfo = NULL;
	WsEventThreadContextH threadcntx = NULL;
	WsContextH soapCntx = ws_get_soap_context(soap);
	pthread_t eventsender;
	pthread_attr_t pattrs;
	int r;
	if ((r = pthread_attr_init(&pattrs)) != 0) {
		debug("pthread_attr_init failed = %d", r);
		return;
	}
	if ((r = pthread_attr_setdetachstate(&pattrs,
					     PTHREAD_CREATE_DETACHED)) !=0) {
		debug("pthread_attr_setdetachstate = %d", r);
		return;
	}
	pthread_mutex_lock(&soap->lockSubs);
	lnode_t *node = list_first(soapCntx->subscriptionMemList);
	while(node) {
		subsInfo = (WsSubscribeInfo *)node->list_data;
		pthread_mutex_lock(&subsInfo->notificationlock);
#if 0
		debug("subscription %s : event sent last time = %d, heartbeat= %ld, heartbeatcountdown = %ld, pending events = %d",
			subsInfo->subsId, 	subsInfo->eventSentLastTime, subsInfo->heartbeatInterval, subsInfo->heartbeatCountdown, subsInfo->flags & WSMAN_SUBSCRIPTION_NOTIFICAITON_PENDING);
#endif
		if(subsInfo->flags & WSMAN_SUBSCRIBEINFO_UNSUBSCRIBE) {
			goto LOOP;
		}
		if(time_expired(subsInfo->expires)) {

			goto LOOP;

		}
		if(subsInfo->heartbeatInterval == 0 || subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_PULL) {
			goto LOOP;
		}
		subsInfo->heartbeatCountdown -= 1000;
		if(subsInfo->heartbeatCountdown > 0) {
			goto LOOP;
		}
		if(subsInfo->eventSentLastTime) {
			subsInfo->eventSentLastTime = 0;
		}
		else {
			debug("one heartbeat document created for %s", subsInfo->subsId);
			if((subsInfo->flags & WSMAN_SUBSCRIPTION_NOTIFICAITON_PENDING) == 0) {
				threadcntx = ws_create_event_context(soap, subsInfo, NULL);
				if(pthread_create(&eventsender, &pattrs, wse_heartbeat_sender, threadcntx) == 0)
					subsInfo->flags |= WSMAN_SUBSCRIPTION_NOTIFICAITON_PENDING;
			}
		}
		subsInfo->heartbeatCountdown = subsInfo->heartbeatInterval;
LOOP:
		pthread_mutex_unlock(&subsInfo->notificationlock);
		node = list_next(soapCntx->subscriptionMemList, node);
	}
	pthread_mutex_unlock(&soap->lockSubs);
}

static int wse_send_notification(WsEventThreadContextH cntx, WsXmlDocH outdoc, WsSubscribeInfo *subsInfo, unsigned char acked)
{
	int retVal = 0;
	WsManClient *notificationSender = wsmc_create_from_uri(subsInfo->epr_notifyto);
	if(subsInfo->contentEncoding)
		wsmc_set_encoding(notificationSender, subsInfo->contentEncoding);
	if(subsInfo->username)
		wsman_transport_set_userName(notificationSender, subsInfo->username);
	if(subsInfo->password)
		wsman_transport_set_password(notificationSender, subsInfo->password);
	if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTP_BASIC_TYPE) {
	}
	else if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTP_DIGEST_TYPE) {
	}
	else if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTPS_BASIC_TYPE) {
		wsman_transport_set_verify_peer(notificationSender, 0);
	}
	else if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTPS_DIGEST_TYPE) {
		wsman_transport_set_verify_peer(notificationSender, 0);
	}
	else if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_TYPE) {
		wsman_transport_set_verify_peer(notificationSender, 1);
		wsman_transport_set_certhumbprint(notificationSender, subsInfo->certificate_thumbprint);
	}
	else if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_BASIC_TYPE) {
		wsman_transport_set_verify_peer(notificationSender, 1);
		wsman_transport_set_certhumbprint(notificationSender, subsInfo->certificate_thumbprint);
	}
	else if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_DIGEST_TYPE) {
		wsman_transport_set_verify_peer(notificationSender, 1);
		wsman_transport_set_certhumbprint(notificationSender, subsInfo->certificate_thumbprint);
	}
	else if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTPS_SPNEGO_KERBEROS_TYPE) {
	}
	else if(subsInfo->deliveryAuthType ==
		WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_SPNEGO_KERBEROS_TYPE) {
	}
	else { //WSMAN_SECURITY_PROFILE_HTTP_SPNEGO_KERBEROS_TYPE
	}
	wsmc_transport_init(notificationSender, NULL);
	wsman_send_request(notificationSender, outdoc);
	if(acked) {
		retVal = WSE_NOTIFICATION_NOACK;
		WsXmlDocH ackdoc = wsmc_build_envelope_from_response(notificationSender);
		if(ackdoc) {
			WsXmlNodeH node = ws_xml_get_soap_header(ackdoc);
			WsXmlNodeH srcnode = ws_xml_get_soap_header(outdoc);
			WsXmlNodeH temp = NULL;
			srcnode = ws_xml_get_child(srcnode, 0, XML_NS_ADDRESSING, WSA_MESSAGE_ID);
			if(node) {
				temp = ws_xml_get_child(node, 0, XML_NS_ADDRESSING, WSA_RELATES_TO);
				if(temp) {
					if(!strcasecmp(ws_xml_get_node_text(srcnode),
						ws_xml_get_node_text(temp))) {
						node = ws_xml_get_child(node, 0, XML_NS_ADDRESSING, WSA_ACTION);
						if(!strcasecmp(ws_xml_get_node_text(node), WSMAN_ACTION_ACK))
							retVal = 0;
					}

				}
			}
			ws_xml_destroy_doc(ackdoc);
		}
	}
	wsmc_release(notificationSender);
	return retVal;
}


static void * wse_event_sender(void * thrdcntx, unsigned char flag)
{
	char uuidBuf[50];
	WsXmlNodeH header;
	if(thrdcntx == NULL) return NULL;
	WsEventThreadContextH threadcntx = (WsEventThreadContextH)thrdcntx;
	WsSubscribeInfo * subsInfo = threadcntx->subsInfo;
	if(flag == 1)
		debug("wse_notification_sender for %s stated", subsInfo->subsId);
	else
		debug("wse_heartbeat_sender for %s started", subsInfo->subsId);
	WsXmlDocH notificationDoc = NULL;
	pthread_mutex_lock(&subsInfo->notificationlock);
	if(flag == 1)
		subsInfo->eventSentLastTime = 1;
	if(!(subsInfo->flags & WSMAN_SUBSCRIBEINFO_UNSUBSCRIBE) &&
		!time_expired(subsInfo->expires)) {
		if(flag) {
			notificationDoc = threadcntx->outdoc;
		}
		else {
	 		notificationDoc = ws_xml_duplicate_doc(subsInfo->heartbeatDoc);
			header = ws_xml_get_soap_header(notificationDoc);
			generate_uuid(uuidBuf, sizeof(uuidBuf), 0);
			ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_MESSAGE_ID,uuidBuf);
		}
		if (subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_EVENTS  ||
			subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_PUSHWITHACK){
			if(wse_send_notification(threadcntx, notificationDoc, subsInfo, 1) == WSE_NOTIFICATION_NOACK)
				subsInfo->flags |= WSMAN_SUBSCRIPTION_CANCELLED;
		}
		else
			wse_send_notification(threadcntx, notificationDoc, subsInfo, 0);
	}
	ws_xml_destroy_doc(notificationDoc);
	subsInfo->flags &= ~WSMAN_SUBSCRIPTION_NOTIFICAITON_PENDING;
	debug("[ wse_notification_sender thread for %s quit! ]",subsInfo->subsId);
	pthread_mutex_unlock(&subsInfo->notificationlock);
	u_free(thrdcntx);
	return NULL;
}

void * wse_heartbeat_sender(void *thrdcntx)
{
	return wse_event_sender(thrdcntx, 0);
}

void *wse_notification_sender(void *thrdcntx)
{
	return wse_event_sender(thrdcntx, 1);
}

void wse_notification_manager(void * cntx)
{
	int retVal;
	WsSubscribeInfo * subsInfo = NULL;
	WsXmlDocH notificationDoc =NULL;
	WsXmlNodeH header = NULL;
	WsXmlNodeH body = NULL;
	WsXmlNodeH node = NULL;
	WsXmlNodeH eventnode = NULL;
	WsXmlNodeH temp = NULL;
	lnode_t *subsnode = NULL;
	WsEventThreadContextH threadcntx = NULL;
	WsContextH contex = (WsContextH)cntx;
	SoapH soap = contex->soap;
	WsContextH soapCntx = ws_get_soap_context(soap);
	pthread_t eventsender;
	pthread_attr_t pattrs;
	char uuidBuf[50];
	int r;
	if ((r = pthread_attr_init(&pattrs)) != 0) {
		debug("pthread_attr_init failed = %d", r);
		return;
	}
	if ((r = pthread_attr_setdetachstate(&pattrs,
					     PTHREAD_CREATE_DETACHED)) !=0) {
		debug("pthread_attr_setdetachstate = %d", r);
		return;
	}
	pthread_mutex_lock(&soap->lockSubs);
	subsnode = list_first(soapCntx->subscriptionMemList);
	while(subsnode) {
		subsInfo = (WsSubscribeInfo *)subsnode->list_data;
		pthread_mutex_lock(&subsInfo->notificationlock);
		threadcntx = ws_create_event_context(soap, subsInfo, NULL);
		if(((subsInfo->flags & WSMAN_SUBSCRIBEINFO_UNSUBSCRIBE) ||
			subsInfo->flags & WSMAN_SUBSCRIPTION_CANCELLED ||
			time_expired(subsInfo->expires)) &&
			((subsInfo->flags & WSMAN_SUBSCRIPTION_NOTIFICAITON_PENDING ) == 0)) {
			lnode_t *nodetemp = list_delete2(soapCntx->subscriptionMemList, subsnode);
			soap->subscriptionOpSet->delete_subscription(soap->uri_subsRepository, subsInfo->subsId);
			soap->eventpoolOpSet->clear(subsInfo->subsId, delete_notification_info);
			if(!(subsInfo->flags & WSMAN_SUBSCRIBEINFO_UNSUBSCRIBE) && subsInfo->cancel)
				subsInfo->cancel(threadcntx);
			if(subsInfo->flags & WSMAN_SUBSCRIBEINFO_UNSUBSCRIBE)
				debug("Unsubscribed!uuid:%s deleted", subsInfo->subsId);
			else if(subsInfo->flags & WSMAN_SUBSCRIPTION_CANCELLED)
				debug("Cancelled! uuid:%s deleted", subsInfo->subsId);
			else
				debug("Expired! uuid:%s deleted", subsInfo->subsId);
			destroy_subsinfo(subsInfo);
			lnode_destroy(subsnode);
			u_free(threadcntx);
			subsnode = nodetemp;
			continue;
		}
		if(subsInfo->eventpoll) { //poll the events
			retVal = subsInfo->eventpoll(threadcntx);
			if(retVal == WSE_NOTIFICATION_EVENTS_PENDING) {
				goto LOOP;
			}
		}
		if(subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_PULL)
			goto LOOP;
		WsNotificationInfoH notificationInfo = NULL;
		if(soap->eventpoolOpSet->remove(subsInfo->subsId, &notificationInfo) ) // to get the event and delete it from the event source
			goto LOOP;
		if(subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_PULL) goto LOOP;
		notificationDoc = ws_xml_duplicate_doc(subsInfo->templateDoc);
		header = ws_xml_get_soap_header(notificationDoc);
		body = ws_xml_get_soap_body(notificationDoc);
		if(notificationInfo->headerOpaqueData) {
			temp = ws_xml_get_doc_root(notificationInfo->headerOpaqueData);
			ws_xml_duplicate_tree(header, temp);
		}
		if(subsInfo->deliveryMode == WS_EVENT_DELIVERY_MODE_EVENTS) {
			ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_ACTION, WSEVENT_DELIVERY_MODE_EVENTS);
			generate_uuid(uuidBuf, sizeof(uuidBuf), 0);
			ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_MESSAGE_ID,uuidBuf);
			eventnode = ws_xml_add_child(body, XML_NS_WS_MAN, WSM_EVENTS, NULL);
			while(notificationInfo) {
				temp = ws_xml_add_child(eventnode, XML_NS_WS_MAN, WSM_EVENT, NULL);
				if(notificationInfo->EventAction)  {
					ws_xml_add_node_attr(temp, XML_NS_WS_MAN, WSM_ACTION, notificationInfo->EventAction);
				}
				else {
					ws_xml_add_node_attr(temp, XML_NS_WS_MAN, WSM_ACTION, WSMAN_ACTION_EVENT);
				}
				if(temp) {
					node = ws_xml_get_doc_root(notificationInfo->EventContent);
					ws_xml_duplicate_children(temp, node);
				}
				delete_notification_info(notificationInfo);
				soap->eventpoolOpSet->remove(subsInfo->subsId, &notificationInfo);
			}
		}
		else{
			generate_uuid(uuidBuf, sizeof(uuidBuf), 0);
			ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_MESSAGE_ID,uuidBuf);
			if(notificationInfo->EventAction)
				ws_xml_add_child(header, XML_NS_WS_MAN, WSM_ACTION, notificationInfo->EventAction);
			else
				ws_xml_add_child(header, XML_NS_WS_MAN, WSM_ACTION, WSMAN_ACTION_EVENT);
			node = ws_xml_get_doc_root(notificationInfo->EventContent);
			ws_xml_duplicate_children(body, node);
			delete_notification_info(notificationInfo);
		}
		if(subsInfo->deliveryMode != WS_EVENT_DELIVERY_MODE_PULL) {
			if((subsInfo->flags & WSMAN_SUBSCRIPTION_NOTIFICAITON_PENDING) == 0) {
				WsEventThreadContextH threadcntx2 = ws_create_event_context(soap, subsInfo, notificationDoc);
				if(pthread_create(&eventsender, &pattrs, wse_notification_sender, threadcntx2) == 0) {
					subsInfo->flags |= WSMAN_SUBSCRIPTION_NOTIFICAITON_PENDING;
				}
				else {
					debug("thread created for %s failed![ %s ]", subsInfo->subsId, strerror(errno));
				}
			}
		}

LOOP:
		if(threadcntx)
			u_free(threadcntx);
		pthread_mutex_unlock(&subsInfo->notificationlock);
		subsnode = list_next(soapCntx->subscriptionMemList, subsnode);
	}
	pthread_mutex_unlock(&soap->lockSubs);
}



#endif


WsContextH
ws_get_soap_context(SoapH soap)
{
	return soap->cntx;
}


int
ws_remove_context_val(WsContextH cntx, char *name)
{
	int retVal = 1;
	if (cntx && name) {
		hnode_t        *hn;
		u_lock(cntx->soap);
		hn = hash_lookup(cntx->entries, name);
		if (hn) {
			debug("Found context entry: %s", name);
			hash_delete_free(cntx->entries, hn);
			retVal = 0;
		}
		u_unlock(cntx->soap);
	}
	return retVal;
}

int
ws_set_context_ulong_val(WsContextH cntx,
			 char *name,
			 unsigned long val)
{
	int retVal = set_context_val(cntx, name, &val, sizeof(unsigned long),
			0, WS_CONTEXT_TYPE_ULONG);
	return retVal;
}


int
ws_set_context_xml_doc_val(WsContextH cntx,
			   char *name,
			   WsXmlDocH val)
{
	cntx->indoc = val;
	return 0;
}

WsContextH
ws_create_ep_context(SoapH soap,
		     WsXmlDocH doc)
{
	WsContextH      cntx = ws_create_context(soap);
	if (cntx)
		ws_set_context_xml_doc_val(cntx, WSFW_INDOC, doc);
	return cntx;
}


int
ws_destroy_context(WsContextH cntx)
{
	int             retVal = 1;
	if (cntx && cntx->owner) {
		ws_clear_context_entries(cntx);
		ws_clear_context_enuminfos(cntx);
		ws_serializer_cleanup(cntx->serializercntx);
		if(cntx->subscriptionMemList) {
			list_destroy_nodes(cntx->subscriptionMemList);
			list_destroy(cntx->subscriptionMemList);
		}
		u_free(cntx);
		retVal = 0;
	}
	return retVal;
}


hnode_t*
create_context_entry(hash_t * h,
		     char *name,
		     void *val)
{
	char           *key = u_strdup(name);
	hnode_t        *hn = hnode_create(val);
	hash_insert(h, hn, (void *) key);
	return hn;
}

SoapH
ws_context_get_runtime(WsContextH cntx)
{
	SoapH soap = NULL;
	if (cntx)
		soap = cntx->soap;
	return soap;
}


void *
get_context_val(WsContextH cntx, char *name)
{
	char           *val = NULL;
	if (cntx && name) {
		u_lock(cntx->soap);
		if (cntx->entries) {
			hnode_t *hn = hash_lookup(cntx->entries, name);
			if (hn)
				val = hnode_get(hn);
		}
		u_unlock(cntx->soap);
	}
	return val;
}


void *
ws_get_context_val(WsContextH cntx, char *name, int *size)
{
	return get_context_val(cntx, name);
}


unsigned long
ws_get_context_ulong_val(WsContextH cntx, char *name)
{
	void  *ptr = get_context_val(cntx, name);
	if (ptr != NULL)
		return *((unsigned long *) ptr);
	return 0;
}


SoapOpH
soap_create_op(SoapH soap,
	       char *inboundAction,
	       char *outboundAction, //optional
	       char *role,
	       SoapServiceCallback callbackProc,
	       void *callbackData,
	       unsigned long flags)
{
	SoapDispatchH  disp = NULL;
	op_t           *entry = NULL;

	if ((disp = wsman_dispatch_create(soap, inboundAction, outboundAction,
			      NULL, //reserved, must be NULL
			      callbackProc, callbackData, flags)) != NULL) {
		entry = create_op_entry(soap, disp, NULL);
	}
	return (SoapOpH) entry;
}



/**
 * Get Operation Document
 * @param op Operation Handle
 * @param inbound Direction flag
 * @return XML Document
 */
WsXmlDocH
soap_get_op_doc(SoapOpH op,
		int inbound)
{
	WsXmlDocH       doc = NULL;
	if (op) {
		op_t           *e = (op_t *) op;
		doc = (!inbound) ? e->out_doc : e->in_doc;
	}
	return doc;
}

WsXmlDocH
soap_detach_op_doc(SoapOpH op,
		   int inbound)
{
	WsXmlDocH       doc = NULL;
	if (op) {
		op_t           *e = (op_t *) op;
		if (!inbound) {
			doc = e->out_doc;
			e->out_doc = NULL;
		} else {
			doc = e->in_doc;
			e->in_doc = NULL;
		}
	}
	return doc;
}

int
soap_set_op_doc(SoapOpH op,
		WsXmlDocH doc,
		int inbound)
{
	int             retVal = 1;
	if (op) {
		op_t           *e = (op_t *) op;
		if (!inbound)
			e->out_doc = doc;
		else
			e->in_doc = doc;
		retVal = 0;
	}
	return retVal;
}


SoapH
soap_get_op_soap(SoapOpH op)
{
	if (op)
		return (SoapH) ((op_t *) op)->dispatch->soap;

	return NULL;
}

void
soap_destroy_op(SoapOpH op)
{
	destroy_op_entry((op_t *) op);
}


op_t *
create_op_entry(SoapH soap,
		SoapDispatchH dispatch,
		WsmanMessage * data)
{
	op_t *entry = (op_t *) u_zalloc(sizeof(op_t));
	if (entry) {
		entry->dispatch = dispatch;
		entry->cntx = ws_create_context(soap);
		entry->data = data;
		// entry->processed_headers = list_create(LISTCOUNT_T_MAX);
	}
	return entry;
}


void
destroy_op_entry(op_t * entry)
{
	SoapH           soap;
	debug("destroy op");
	if (!entry) {
		debug("nothing to destroy...");
		return;
	}
	soap = entry->dispatch->soap;
	if (soap == NULL) {
		goto NULL_SOAP;
	}
	u_lock(soap);
	if (soap->dispatchList && list_contains(soap->dispatchList, &entry->dispatch->node)) {
		list_delete(soap->dispatchList, &entry->dispatch->node);
	}
	u_unlock(soap);

NULL_SOAP:
	destroy_dispatch_entry(entry->dispatch);
	ws_destroy_context(entry->cntx);
#if 0
	list_destroy_nodes(entry->processed_headers);
	list_destroy(entry->processed_headers);
#endif
	u_free(entry);
}

void
destroy_dispatch_entry(SoapDispatchH entry)
{
	int usageCount;
	list_t *dlist;
	if (!entry) {
		return;
	}

	u_lock(entry->soap);
	entry->usageCount--;
	usageCount = entry->usageCount;
	dlist = entry->soap->dispatchList;
	if (!usageCount && dlist != NULL &&
			list_contains(dlist, &entry->node)) {
		lnode_t *n = list_delete(dlist, &entry->node);
		lnode_destroy(n);
	}
	u_unlock(entry->soap);

	if (!usageCount) {
		if (entry->inboundFilterList) {
			list_destroy_nodes(entry->inboundFilterList);
			list_destroy(entry->inboundFilterList);
		}
		if (entry->outboundFilterList) {
			list_destroy_nodes(entry->outboundFilterList);
			list_destroy(entry->outboundFilterList);
		}

		u_free(entry->inboundAction);
		u_free(entry->outboundAction);

		u_free(entry);
	}
}


void
soap_destroy(SoapH soap)
{
	if (soap == NULL )
		return;

	if (soap->dispatcherProc)
		soap->dispatcherProc(soap->cntx, soap->dispatcherData, NULL);

	if (soap->dispatchList) {
		while (!list_isempty(soap->dispatchList)) {
			destroy_dispatch_entry(
				(SoapDispatchH)list_first(soap->dispatchList));
		}
		list_destroy(soap->dispatchList);
	}

	if (soap->processedMsgIdList) {
		while (!list_isempty(soap->processedMsgIdList)) {
			lnode_t *node = list_del_first(soap->processedMsgIdList);
			u_free(node->list_data);
			lnode_destroy(node);
		}
		list_destroy(soap->processedMsgIdList);
	}


	if (soap->inboundFilterList) {
		list_destroy_nodes(soap->inboundFilterList);
		list_destroy(soap->inboundFilterList);
	}

	if (soap->outboundFilterList) {
		list_destroy_nodes(soap->outboundFilterList);
		list_destroy(soap->outboundFilterList);
	}
	ws_xml_parser_destroy();

	ws_destroy_context(soap->cntx);
	u_free(soap);

	return;
}


void
wsman_status_init(WsmanStatus * status)
{
	status->fault_code = 0;
	status->fault_detail_code = 0;
	status->fault_msg = NULL;
}

int
wsman_check_status(WsmanStatus * status)
{
	return status->fault_code;
}
