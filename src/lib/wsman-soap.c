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
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#define _GNU_SOURCE

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"

#include "wsman-dispatcher.h"
#include "wsman-xml-serializer.h"
#include "wsman-xml-serialize.h"
#include "wsman-client.h"
#include "wsman-soap-envelope.h"
#include "wsman-faults.h"
#include "wsman-soap-message.h"


WsXmlNsData     g_wsNsData[] =
{
	{XML_NS_SOAP_1_2, "s"},
	{XML_NS_ADDRESSING, "wsa"},
	{XML_NS_EVENTING, "wse"},
	{XML_NS_ENUMERATION, "wsen"},
	{XML_NS_SCHEMA_INSTANCE, "xsi"},
	{XML_NS_CIM_SCHEMA, "cim"},
	{XML_NS_WS_MAN_CAT, "cat"},
	{XML_NS_WSMAN_ID, "wsmid"},
	{XML_NS_XML_SCHEMA, "xs"},
	{XML_NS_WS_MAN, "wsman"},
	{XML_NS_CIM_BINDING, "wsmb"},
	{XML_NS_OPENWSMAN, "owsman"},
	{XML_NS_TRANSFER, "wxf"},
	{NULL, NULL}
};

WsManDialectData g_wsDialectData[] =
{
	{WSM_WQL_FILTER_DIALECT, "wql"},
	{WSM_SELECTOR_FILTER_DIALECT, "selector"},
	{NULL, NULL}
};


static int
set_context_val(WsContextH cntx,
		char *name,
		void *val,
		int size,
		int no_dup,
		unsigned long type);

callback_t     *
make_callback_entry(SoapServiceCallback proc,
		    void *data,
		    list_t * list_to_add)
{

	callback_t     *entry =
	(callback_t *) u_malloc(sizeof(callback_t));
	debug("make new callback entry");
	if (entry) {
		lnode_init(&entry->node, data);
		entry->proc = proc;
		if (list_to_add)
			list_append(list_to_add, &entry->node);
	}
	return entry;
}

static void
free_hentry_func(hnode_t * n, void *arg)
{
	u_free(hnode_getkey(n));
	u_free(n);
}


void
ws_initialize_context(WsContextH cntx,
		      SoapH soap)
{
	cntx->entries = hash_create(HASHCOUNT_T_MAX, 0, 0);
	hash_set_allocator(cntx->entries, NULL, free_hentry_func, NULL);
	cntx->enuninfos = hash_create(HASHCOUNT_T_MAX, 0, 0);
	hash_set_allocator(cntx->enuninfos, NULL, free_hentry_func, NULL);
//	cntx->last_get_name_idx = -1;
	cntx->owner = 1;
	cntx->soap = soap;
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
	SoapH           soap = (SoapH) u_zalloc(sizeof(*soap));

	if (soap == NULL) {
		error("Could not alloc memory");
		return NULL;
	}
	//fw->dispatchList.listOwner = fw;
	soap->cntx = ws_create_context(soap);
	soap->inboundFilterList = list_create(LISTCOUNT_T_MAX);
	soap->outboundFilterList = list_create(LISTCOUNT_T_MAX);
	soap->dispatchList = list_create(LISTCOUNT_T_MAX);
	soap->responseList = list_create(LISTCOUNT_T_MAX);
	soap->processedMsgIdList = list_create(LISTCOUNT_T_MAX);
	soap->WsSerializerAllocList = list_create(LISTCOUNT_T_MAX);
//	soap->enumIdleTimeout = enumIdleTimeout;
	u_init_lock(soap);
	ws_xml_parser_initialize(soap, g_wsNsData);
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


WsContextH
ws_create_runtime(list_t * interfaces)
{
	SoapH           soap = ws_soap_initialize();
	WsManDispatcherInfo *dispInfo;
	int             size;
	lnode_t        *node;
	if (soap == NULL) {
		error("Could not initialize soap");
		return NULL;
	}
	if (interfaces == NULL) {
		error("NULL interfaces");
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
			soap_destroy_fw(soap);
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
	int             retVal = 0;
	int             i;

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
			WsDispatchInterfaceInfo * wsInterface,
			WsDispatchEndPointInfo * ep,
			WsManDispatcherInfo * dispInfo)
{
	SoapDispatchH   disp = NULL;
	unsigned long   flags = SOAP_CUSTOM_DISPATCHER;
	SoapServiceCallback callbackProc = NULL;
	SoapH           soap = ws_context_get_runtime(cntx);
	char           *action = NULL;
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
		debug("Registering endpoint for Pull Raw");
		action = ep->inAction;
		callbackProc = wsenum_pull_raw_stub;
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

	if (callbackProc != NULL && (disp = wsman_dispatch_create(soap, action,
			NULL, NULL, callbackProc, ep, flags))) {
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





		//   ENDPOINTS STUBS



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
		doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1),
					    WSMAN_INTERNAL_ERROR, 0, NULL);
	} else {
		doc = wsman_create_response_envelope(cntx,
			soap_get_op_doc(op, 1), NULL);
		ws_serialize(cntx, ws_xml_get_soap_body(doc), data, typeInfo,
			WSMID_IDENTIFY_RESPONSE, (char *) info->data, NULL, 1);
		ws_serializer_free_mem(cntx, data, typeInfo);
	}

	if (doc) {
		soap_set_op_doc(op, doc, 0);
	} else {
		error("Response doc invalid");
	}

	ws_serializer_free_all(cntx);
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


	void  *data = ws_deserialize(cntx, _body, typeInfo,
					ws_xml_get_node_local_name(_r),
			    	(char *) info->data, NULL, 0, 0);

	if ((retVal = endPoint(cntx, data, &outData, &status, opaqueData))) {
		doc = wsman_generate_fault(cntx, _doc, status.fault_code,
					   status.fault_detail_code, NULL);
	} else {
		doc = wsman_create_response_envelope(cntx, _doc, NULL);
		if (outData) {
			ws_serialize(cntx, ws_xml_get_soap_body(doc), outData,
				     typeInfo, TRANSFER_PUT_RESP,
                     (char *) info->data, NULL, 1);
			ws_serializer_free_mem(cntx, outData, typeInfo);
		}
	}

	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	ws_serializer_free_all(cntx);
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
		doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1),
					 WSMAN_INVALID_SELECTORS, 0, NULL);
	} else {
		debug("Creating Response doc");
		doc = wsman_create_response_envelope(cntx,
				soap_get_op_doc(op, 1), NULL);
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
		doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1),
					 WSMAN_INVALID_SELECTORS, 0, NULL);
	} else {
		debug("Creating Response doc");
		doc = wsman_create_response_envelope(cntx,
			soap_get_op_doc(op, 1), NULL);

		ws_serialize(cntx, ws_xml_get_soap_body(doc), data, typeInfo,
			 TRANSFER_GET_RESP, (char *) info->data, NULL, 1);
		ws_serializer_free_mem(cntx, data, typeInfo);
	}

	if (doc) {
		debug("Setting operation document");
		soap_set_op_doc(op, doc, 0);
	} else {
		warning("Response doc invalid");
	}

	ws_serializer_free_all(cntx);
	ws_destroy_context(cntx);
	return 0;
}




		//    ENUMERATION STUFF


#define ENUM_EXPIRED(enuminfo, mytime) \
	((enumInfo->expires > 0) &&        \
	(enumInfo->expires > mytime))


static int
wsman_verify_enum_info(SoapOpH op,
		       WsEnumerateInfo * enumInfo,
		       WsXmlDocH doc,
		       WsmanStatus * status)
{

	op_t           *_op = (op_t *) op;
	WsmanMessage   *msg = (WsmanMessage *) _op->data;
	WsXmlNodeH  header = ws_xml_get_soap_header(doc);
	char *to =  ws_xml_get_node_text(
			ws_xml_get_child(header, 0, XML_NS_ADDRESSING, WSA_TO));
	char *uri=  ws_xml_get_node_text(
			ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_RESOURCE_URI));

	if (strcmp(enumInfo->epr_to, to) != 0 ||
			strcmp(enumInfo->epr_uri, uri) != 0 ) {
		status->fault_code = WSA_MESSAGE_INFORMATION_HEADER_REQUIRED;
		status->fault_detail_code = 0;
		debug("verifying enumeration context: ACTUAL  uri: %s, to: %s", uri, to);
		debug("verifying enumeration context: SHOULD uri: %s, to: %s", enumInfo->epr_uri, enumInfo->epr_to);
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
	if (create_context_entry(cntx->enuninfos, enumInfo->enumId, enumInfo)) {
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
	char            *enumId = NULL;
	WsXmlNodeH      node = ws_xml_get_soap_body(doc);

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
	hn = hash_lookup(cntx->enuninfos, enumId);
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
remove_locked_enuminfo(WsContextH cntx,
                       WsEnumerateInfo * enumInfo)
{
	u_lock(cntx->soap);
	if (!(enumInfo->flags & WSMAN_ENUMINFO_INWORK_FLAG)) {
		error("locked enuminfo unlocked");
		u_unlock(cntx->soap);
		return;
	}
	hash_delete_free(cntx->enuninfos,
	             hash_lookup(cntx->enuninfos, enumInfo->enumId));
	u_unlock(cntx->soap);
}

static void
wsman_set_enum_expiretime(WsXmlNodeH  node,
                    WsEnumerateInfo * enumInfo,
                    WsmanFaultCodeType *fault_code)
{
	struct timeval  tv;
	time_t timeout;
	char *text;
	WsXmlNodeH  child;
	XML_DATETIME tmx;
	gettimeofday(&tv, NULL);
	child = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_EXPIRES);
	if (child == NULL) {
		debug("No wsen:Expires");
		enumInfo->expires = 0;
		return;
	}
	text = ws_xml_get_node_text(child);
	if (text == NULL) {
		*fault_code = WSEN_INVALID_EXPIRATION_TIME;
		return;
	}
	debug("wsen:Expires = %s", text);
	if (text[0] == 'P') {
		//  xml duration
		if (ws_deserialize_duration(text, &timeout)) {
			*fault_code = WSEN_INVALID_EXPIRATION_TIME;
			goto DONE;
		}
		debug("timeout = %ul", timeout);
		enumInfo->expires = (tv.tv_sec + timeout) * 1000 +
			tv.tv_usec / 1000;
		goto DONE;
	}

	// timeout is XML datetime type
	if (ws_deserialize_datetime(text, &tmx)) {
		*fault_code = WSEN_UNSUPPORTED_EXPIRATION_TYPE;
		goto DONE;
	}
	timeout = mktime(&(tmx.tm)) + 60*tmx.tz_min;
	timeout -= (time_t)__timezone;
	enumInfo->expires = timeout * 1000;
	debug("enumInfo->expires = %ul", enumInfo->expires);
DONE:
	return;
}



static WsXmlDocH
create_enum_info(SoapOpH op,
		 WsContextH epcntx,
              	 WsXmlDocH indoc,
		 WsEnumerateInfo **eInfo)
{
	WsXmlNodeH  node = ws_xml_get_soap_body(indoc);
	WsXmlNodeH  header = ws_xml_get_soap_header(indoc);
	WsXmlDocH outdoc = NULL;
	WsEnumerateInfo *enumInfo;
	op_t           *_op = (op_t *) op;
	WsmanMessage   *msg = (WsmanMessage *) _op->data;
	WsmanFaultCodeType fault_code = WSMAN_RC_OK;
	WsmanFaultDetailType fault_detail_code = WSMAN_DETAIL_OK;
	char *uri, *to;

	enumInfo = (WsEnumerateInfo *)u_zalloc(sizeof (WsEnumerateInfo));
	if (enumInfo == NULL) {
		error("No memory");
		fault_detail_code = WSMAN_INTERNAL_ERROR;
		goto DONE;
	}
	enumInfo->releaseproc = wsman_get_release_endpoint(epcntx, indoc);
	to = ws_xml_get_node_text(
			ws_xml_get_child(header, 0, XML_NS_ADDRESSING, WSA_TO));
	uri =  ws_xml_get_node_text(
			ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_RESOURCE_URI));

	enumInfo->epr_to = u_strdup(to);
	enumInfo->epr_uri = u_strdup(uri);
	node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE);
	wsman_set_enum_expiretime(node, enumInfo, &fault_code);
	if (fault_code != WSMAN_RC_OK) {
		fault_detail_code = WSMAN_DETAIL_EXPIRATION_TIME;
		goto DONE;
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
		outdoc = wsman_generate_fault(epcntx, indoc,
			 fault_code, fault_detail_code, NULL);
		u_free(enumInfo);
	} else {
		*eInfo = enumInfo;
	}
	return outdoc;
}

static void destroy_filter(filter_t *filter)
{
	if (filter->epr) {
		//u_free(filter->epr);
	}

}

static void
destroy_enuminfo(WsEnumerateInfo * enumInfo)
{
	u_free(enumInfo->auth_data.username);
	u_free(enumInfo->auth_data.password);
	u_free(enumInfo->epr_to);
	u_free(enumInfo->epr_uri);
	if (enumInfo->filter)
		destroy_filter(enumInfo->filter);
	u_free(enumInfo);
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
	WsEnumerateInfo *enumInfo;
	WsmanStatus     status;
	WsXmlNodeH      resp_node;
	WsXmlNodeH      body;
	WsContextH      soapCntx;
	SoapH           soap = soap_get_op_soap(op);

	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;
	WsEndPointEnumerate endPoint =
			(WsEndPointEnumerate)ep->serviceEndPoint;

	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	WsContextH      epcntx;

	epcntx = ws_create_ep_context(soap, _doc);
	enumInfo = (WsEnumerateInfo *)u_zalloc(sizeof (WsEnumerateInfo));
	wsman_status_init(&status);
	doc = create_enum_info(op, epcntx, _doc, &enumInfo);
	if (doc != NULL) {
		// wrong enum elements met. Fault message generated
		goto DONE;
	}

	if (endPoint && (retVal = endPoint(epcntx, enumInfo, &status, opaqueData))) {
                debug("enumeration fault");
		doc = wsman_generate_fault(epcntx, _doc,
			 status.fault_code, status.fault_detail_code, NULL);
		destroy_enuminfo(enumInfo);
		goto DONE;
	}
	if (enumInfo->pullResultPtr) {
		doc = enumInfo->pullResultPtr;
		enumInfo->index++;
	} else {
		doc = wsman_create_response_envelope(epcntx, _doc, NULL);
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
	if (enumInfo->index == enumInfo->totalItems) {
		ws_serialize_str(epcntx, resp_node, NULL,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
		ws_serialize_str(epcntx, resp_node,
			       NULL, XML_NS_WS_MAN, WSENUM_END_OF_SEQUENCE, 0);
		destroy_enuminfo(enumInfo);
	} else {
		ws_serialize_str(epcntx, resp_node, enumInfo->enumId,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
		insert_enum_info(soapCntx, enumInfo);
	}

DONE:
	if (doc) {
		soap_set_op_doc(op, doc, 0);
	}
	ws_serializer_free_all(epcntx);
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
		doc = wsman_generate_fault(soapCntx, _doc,
			status.fault_code, status.fault_detail_code, NULL);

	} else {
		if (endPoint && (retVal = endPoint(soapCntx,
						enumInfo, &status, opaqueData))) {
			error("endPoint error");
			doc = wsman_generate_fault(soapCntx, _doc,
				WSMAN_INTERNAL_ERROR,
				OWSMAN_DETAIL_ENDPOINT_ERROR, NULL);
			unlock_enuminfo(soapCntx, enumInfo);
		} else {
			doc = wsman_create_response_envelope(
						soapCntx, _doc, NULL);
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
//	char            cntxName[64];
	int             retVal = 0;
	WsXmlDocH       doc = NULL;
	char           *enumId = NULL;
	int locked = 0;

	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	WsEnumerateInfo *enumInfo;

	wsman_status_init(&status);
	enumInfo = get_locked_enuminfo(soapCntx, _doc,
		op, WSENUM_PULL, &status);

	if (enumInfo == NULL) {
		doc = wsman_generate_fault(soapCntx, _doc,
			status.fault_code, status.fault_detail_code, NULL);
		goto DONE;
	}
	locked = 1;
	if ((retVal = endPoint(ws_create_ep_context(soap, _doc),
						enumInfo, &status, opaqueData))) {
		doc = wsman_generate_fault(soapCntx, _doc,
			 status.fault_code, status.fault_detail_code, NULL);
		goto DONE;
	}
	enumInfo->index++;
	doc = wsman_create_response_envelope(soapCntx, _doc, NULL);
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
		WsXmlNodeH      itemsNode = ws_xml_add_child(node,
				    XML_NS_ENUMERATION, WSENUM_ITEMS, NULL);
		ws_serialize(soapCntx, itemsNode, enumInfo->pullResultPtr,
			 typeInfo, ep->respName, (char *) ep->data, NULL, 1);
		if (enumId) {
			ws_serialize_str(soapCntx, node, enumId,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
		}
		ws_serializer_free_mem(soapCntx,
			enumInfo->pullResultPtr, typeInfo);
	} else {
		/*
		ws_serialize_str(soapCntx, node, NULL,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
			    */
		ws_serialize_str(soapCntx,
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
wsenum_pull_raw_stub(SoapOpH op,
		     void *appData,
			void *opaqueData)
{
	WsmanStatus     status;
	WsXmlDocH       doc = NULL;
	SoapH           soap = soap_get_op_soap(op);
	WsContextH      soapCntx = ws_get_soap_context(soap);
	WsDispatchEndPointInfo *ep = (WsDispatchEndPointInfo *) appData;


	WsEndPointPull  endPoint = (WsEndPointPull) ep->serviceEndPoint;
	int             retVal = 0;
	WsXmlDocH       _doc = soap_get_op_doc(op, 1);
	int locked = 0;
	WsEnumerateInfo *enumInfo;

	wsman_status_init(&status);
	enumInfo = get_locked_enuminfo(soapCntx,
	                               _doc, op, WSENUM_PULL, &status);

	if (enumInfo == NULL) {
		error("Invalid enumeration context...");
		doc = wsman_generate_fault(soapCntx, _doc,
			status.fault_code, status.fault_detail_code, NULL);
		goto cleanup;
	}
	locked = 1;

	if ((retVal = endPoint(ws_create_ep_context(soap, _doc),
						enumInfo, &status, opaqueData))) {
		doc = wsman_generate_fault(soapCntx, _doc,
			 status.fault_code, status.fault_detail_code, NULL);
//		ws_remove_context_val(soapCntx, cntxName);
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

		if (enumInfo->index == enumInfo->totalItems) {
			/*
			ws_serialize_str(soapCntx, response, NULL,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
			    */
			ws_serialize_str(soapCntx, response, NULL,
				XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE, 0);
			remove_locked_enuminfo(soapCntx, enumInfo);
			locked = 0;
			destroy_enuminfo(enumInfo);
		} else  {
			ws_serialize_str(soapCntx, response, enumInfo->enumId,
			    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, 0);
		}
	}
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
	if (hash_isempty(cntx->enuninfos)) {
		u_unlock(cntx->soap);
		return NULL;
	}
	hash_scan_begin(&hs, cntx->enuninfos);
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
//		debug("Enum  (%ul < %ul || %ul < %ul) %d:%d",
//			enumInfo->timeStamp + aeit, mytime,
//			enumInfo->expires, mytime,
//			(enumInfo->timeStamp + aeit > mytime),
//			(enumInfo->expires > mytime));
		if (list == NULL) {
			list = list_create(LISTCOUNT_T_MAX);
		}
		if (list == NULL) {
			u_unlock(cntx->soap);
			error("could not create list");
			return NULL;
		}
		hash_scan_delfree(cntx->enuninfos, hn);
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
	WsmanStatus     status;

	if (list == NULL) {
		return;
	}
	while ((node = list_del_first(list))) {
		enumInfo = (WsEnumerateInfo *)lnode_get(node);
		debug("EnumContext expired : %s", enumInfo->enumId);
		lnode_destroy(node);
		if (enumInfo-> releaseproc) {
			if (enumInfo-> releaseproc(cntx, enumInfo, &status, opaqueData)) {
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










		// SUPPORTING FUNCTIONS


WsContextH
ws_get_soap_context(SoapH soap)
{
	return soap->cntx;
}


/*
  void destroy_context_entry(WS_CONTEXT_ENTRY* entry)
  {
  if ( (entry->options & WS_CONTEXT_FREE_DATA) != 0 )
  u_free(entry->node->list_data);
  u_free(entry->name);
  u_free(entry);
  }
*/


static void
ws_clear_context_entries(WsContextH hCntx)
{
	hash_t         *h;
	if (!hCntx) {
		return;
	}
	h = hCntx->entries;
	hash_free(h);
}

static void
ws_clear_context_enuninfos(WsContextH hCntx)
{
	hash_t         *h;
	if (!hCntx) {
		return;
	}
	h = hCntx->enuninfos;
	hash_free(h);
}
int
ws_remove_context_val(WsContextH cntx,
		      char *name)
{
	int             retVal = 1;
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
	int retVal = set_context_val(cntx, name,
		&val, sizeof(unsigned long), 0, WS_CONTEXT_TYPE_ULONG);
	return retVal;
}


int
ws_set_context_xml_doc_val(WsContextH cntx,
			   char *name,
			   WsXmlDocH val)
{
/*
	int retVal = set_context_val(cntx, name,
		(void *) val, 0, 1, WS_CONTEXT_TYPE_XMLDOC);
	return retVal;
*/
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
		ws_clear_context_enuninfos(cntx);
		u_free(cntx);
		retVal = 0;
	}
	return retVal;
}


hnode_t        *
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
	SoapH           soap = NULL;
	if (cntx)
		soap = cntx->soap;
	return soap;
}


static int
set_context_val(WsContextH cntx,
		char *name,
		void *val,
		int size,
		int no_dup,
		unsigned long type)
{
	int             retVal = 1;
	debug("Setting context value: %s", name);
	if (cntx && name) {
		void           *ptr = val;

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



void           *
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


void           *
ws_get_context_val(WsContextH cntx, char *name, int *size)
{
	return get_context_val(cntx, name);
}


unsigned long
ws_get_context_ulong_val(WsContextH cntx,
			 char *name)
{
	void           *ptr = get_context_val(cntx, name);
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

char           *
soap_get_op_action(SoapOpH op,
		   int inbound)
{
	char           *action = NULL;
	if (op) {
		op_t           *e = (op_t *) op;
		action = (!inbound) ? e->dispatch->outboundAction :
			e->dispatch->inboundAction;
	}
	return action;
}

void
soap_set_op_action(SoapOpH op,
		   char *action,
		   int inbound)
{
	if (op && action) {
		op_t           *e = (op_t *) op;

		if (!inbound) {
			u_free(e->dispatch->outboundAction);
			e->dispatch->outboundAction = u_str_clone(action);
		} else {
			u_free(e->dispatch->inboundAction);
			e->dispatch->inboundAction = u_str_clone(action);
		}
	}
}

unsigned long
soap_get_op_flags(SoapOpH op)
{
	if (op) {
		return ((op_t *) op)->dispatch->flags;
	}
	return 0;
}

SoapH
soap_get_op_soap(SoapOpH op)
{
	if (op)
		return (SoapH) ((op_t *) op)->dispatch->fw;

	return NULL;
}

void
soap_destroy_op(SoapOpH op)
{
	destroy_op_entry((op_t *) op);
}


op_t           *
create_op_entry(SoapH soap,
		SoapDispatchH dispatch,
		WsmanMessage * data)
{
	op_t           *entry = (op_t *) u_zalloc(sizeof(op_t));
	if (entry) {
		entry->dispatch = dispatch;
		entry->cntx = ws_create_context(soap);
		entry->data = data;
		entry->processed_headers = list_create(LISTCOUNT_T_MAX);
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
	soap = entry->dispatch->fw;
	if (soap == NULL) {
		goto NULL_SOAP;
	}
	u_lock(soap);
	if (list_contains(soap->dispatchList, &entry->dispatch->node)) {
		list_delete(soap->dispatchList, &entry->dispatch->node);
	}
	unlink_response_entry(soap, entry);
	u_unlock(soap);
NULL_SOAP:
	destroy_dispatch_entry(entry->dispatch);
	ws_destroy_context(entry->cntx);
	list_destroy_nodes(entry->processed_headers);
	list_destroy(entry->processed_headers);
	u_free(entry);
}

void
destroy_dispatch_entry(SoapDispatchH entry)
{
	int             usageCount;
	if (!entry)
		return;

	u_lock(entry->fw);
	entry->usageCount--;
	usageCount = entry->usageCount;
	if (!usageCount && list_contains(
			entry->fw->dispatchList, &entry->node)) {
		lnode_t *n =
			list_delete(entry->fw->dispatchList, &entry->node);
		lnode_destroy(n);
	}
	u_unlock(entry->fw);

	if (!usageCount && entry->inboundFilterList &&
					entry->outboundFilterList) {
		list_destroy_nodes(entry->inboundFilterList);
		list_destroy(entry->inboundFilterList);
		list_destroy_nodes(entry->outboundFilterList);
		list_destroy(entry->outboundFilterList);

		u_free(entry->inboundAction);
		u_free(entry->outboundAction);

		u_free(entry);
	}
}

WsXmlDocH
ws_get_context_xml_doc_val(WsContextH cntx,
			   char *name)
{
//	return (WsXmlDocH) get_context_val(cntx, name);
	return cntx->indoc;
}


#if 0
void
add_response_entry(SoapH soap, op_t * op)
{
	lnode_t        *n;
	debug("Adding Response Entry");
	u_lock(soap);
	n = lnode_create(op);
	list_append(soap->responseList, n);
	u_unlock(soap);
}
#endif




void
soap_destroy_fw(SoapH soap)
{
	if (soap->dispatcherProc)
		soap->dispatcherProc(soap->cntx, soap->dispatcherData, NULL);

	if (soap->dispatchList) {
		while (!list_isempty(soap->dispatchList)) {
			destroy_dispatch_entry(
				(SoapDispatchH)list_first(soap->dispatchList));
		}
		list_destroy(soap->dispatchList);
	}
	while (!list_isempty(soap->processedMsgIdList)) {
		lnode_t *node = list_del_first(soap->processedMsgIdList);
		u_free(node->list_data);
		lnode_destroy(node);
	}
	list_destroy(soap->processedMsgIdList);

	while (!list_isempty(soap->responseList)) {
		lnode_t        *node = list_del_first(soap->responseList);
		op_t           *entry = (op_t *) node->list_data;
		destroy_op_entry(entry);
		lnode_destroy(node);
	}
	list_destroy(soap->responseList);

	list_destroy_nodes(soap->inboundFilterList);
	list_destroy(soap->inboundFilterList);

	list_destroy_nodes(soap->outboundFilterList);
	list_destroy(soap->outboundFilterList);

	list_destroy_nodes(soap->WsSerializerAllocList);
	list_destroy(soap->WsSerializerAllocList);


	ws_xml_parser_destroy(soap);

	ws_destroy_context(soap->cntx);
	u_free(soap);

	return;
}


void
wsman_status_init(WsmanStatus * s)
{
	s->fault_code = 0;
	s->fault_detail_code = 0;
	s->fault_msg = NULL;
}

int
wsman_check_status(WsmanStatus * s)
{
	return s->fault_code;
}
