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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-client-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"

#include "wsman-dispatcher.h"
#include "wsman-xml-serialize.h"
#include "wsman-faults.h"
#include "wsman-soap-envelope.h"




/**
 * @defgroup Dispatcher Dispatcher
 * @brief SOAP Dispatcher
 *
 * @{
 */


// TBD:? ? ? Should it be SoapH specific
struct __WkHeaderInfo {
	char           *ns;
	char           *name;
};


static int 
is_wk_header(WsXmlNodeH header)
{
	static struct __WkHeaderInfo s_Info[] =
	{
		{XML_NS_ADDRESSING, WSA_TO},
		{XML_NS_ADDRESSING, WSA_MESSAGE_ID},
		{XML_NS_ADDRESSING, WSA_RELATES_TO},
		{XML_NS_ADDRESSING, WSA_ACTION},
		{XML_NS_ADDRESSING, WSA_REPLY_TO},
		{XML_NS_ADDRESSING, WSA_FROM},
		{XML_NS_WS_MAN, WSM_RESOURCE_URI},
		{XML_NS_WS_MAN, WSM_SELECTOR_SET},
		{XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE},
		{XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT},
		{XML_NS_WS_MAN, WSM_FRAGMENT_TRANSFER},
		{NULL, NULL}
	};

	int             i;
	char           *name = ws_xml_get_node_local_name(header);
	char           *ns = ws_xml_get_node_name_ns(header);

	for (i = 0; s_Info[i].name != NULL; i++) {
		if ((ns == NULL && s_Info[i].ns == NULL) ||
				(ns != NULL && s_Info[i].ns != NULL &&
					!strcmp(ns, s_Info[i].ns))) {
			if (!strcmp(name, s_Info[i].name))
				return 1;
		}
	}
	debug("mustUnderstand: %s:%s", !ns ? "null" : ns, name);
	return 0;
}


int 
unlink_response_entry(SoapH soap, op_t * entry)
{
	int             retVal = 0;

	if (soap && entry) {
		int             try = u_try_lock(soap);

		lnode_t        *node = list_first(soap->responseList);
		while (node != NULL) {
			if (entry == (op_t *) node->list_data) {
				list_delete(soap->responseList, node);
				u_free(node);
				retVal = 1;
				break;
			}
			node = list_next(soap->responseList, node);
		}

		if (!try)
			u_unlock(soap);
	}
	return retVal;
}


static void
wsman_generate_op_fault(op_t * op,
			WsmanFaultCodeType faultCode,
			WsmanFaultDetailType faultDetail)
{
	if (op->out_doc) {
		ws_xml_destroy_doc(op->out_doc);
		op->out_doc = NULL;
	}
	if (op->in_doc == NULL) {
		return;
	}
	op->out_doc = wsman_generate_fault(op->cntx, op->in_doc, faultCode,
					   faultDetail, NULL);
	return;
}

void 
wsman_generate_notunderstood_fault( op_t* op, 
		WsXmlNodeH notUnderstoodHeader) 
{
	WsXmlNodeH child;
	WsXmlNodeH header;

	if (op->in_doc == NULL)
		return;
	wsman_generate_op_fault(op,
			SOAP_FAULT_MUSTUNDERSTAND,
			SOAP_DETAIL_HEADER_NOT_UNDERSTOOD);

	if (op->out_doc != NULL) {
		header = ws_xml_get_soap_header(op->out_doc);
		if (header) {
			child = ws_xml_add_child(header, XML_NS_SOAP_1_2, "NotUnderstood", NULL);
			ws_xml_add_qname_attr(child, NULL, "qname", ws_xml_get_node_name_ns(notUnderstoodHeader),
					ws_xml_get_node_local_name(notUnderstoodHeader));
		}
	} else {
		debug("cant generate fault");
	}

	return;
}




static int 
check_for_duplicate_selectors(op_t * op)
{
	WsXmlNodeH      header, node, selector;
	int retval = 0;
	int index = 0;
	hash_t  *h;

	header = wsman_get_soap_header_element(op->dispatch->fw, op->in_doc, NULL, NULL);
	if ( (node = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET) ) == NULL) {
		// No selectors
		return 0;
	}
	h = hash_create(HASHCOUNT_T_MAX, 0, 0);
	if (h == NULL) {
		wsman_generate_op_fault(op, WSMAN_INTERNAL_ERROR,
						OWSMAN_NO_DETAILS);
		error("could not create hash");
		return 1;
	}
			
	while ((selector = ws_xml_get_child(node, index++, XML_NS_WS_MAN, WSM_SELECTOR))) {
		char *attrVal = ws_xml_find_attr_value(selector, NULL, WSM_NAME);
		if (!attrVal) 
			continue;
		if (hash_lookup(h, attrVal)) {
			wsman_generate_op_fault(op, WSMAN_INVALID_SELECTORS,
						WSMAN_DETAIL_DUPLICATE_SELECTORS );
			debug("Selector %s duplicated", attrVal);
			retval = 1;
			break;
		}
		if (!hash_alloc_insert(h, attrVal,
							ws_xml_get_node_text(selector))) {
			wsman_generate_op_fault(op, WSMAN_INTERNAL_ERROR,
						OWSMAN_NO_DETAILS);
			retval = 1;
			error("hash_alloc_insert failed");
			break;
		}
	}
	hash_free_nodes(h);
	hash_destroy(h);
	return retval;
}

static int 
validate_control_headers(op_t * op)
{
	unsigned long   size = 0;
	time_t duration;
	WsXmlNodeH      header;
	WsXmlNodeH      child, maxsize;
	char *mu = NULL;

	header = wsman_get_soap_header_element(
				op->dispatch->fw, op->in_doc, NULL, NULL);
	maxsize = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE);
	mu = ws_xml_find_attr_value(maxsize, XML_NS_SOAP_1_2 , SOAP_MUST_UNDERSTAND);
	if (mu != NULL && strcmp(mu, "true") == 0 ) {
		size = ws_deserialize_uint32(NULL, header,
						0, XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE);
		if (size < WSMAN_MINIMAL_ENVELOPE_SIZE_REQUEST) {
			wsman_generate_op_fault(op, WSMAN_ENCODING_LIMIT,
					 WSMAN_DETAIL_MINIMUM_ENVELOPE_LIMIT);
			return 0;
		}
	}
	child = ws_xml_get_child(header, 0,
				XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT);
	if (child != NULL) {
		char *text =  ws_xml_get_node_text(child);
		char *nsUri = ws_xml_get_node_name_ns(header);
		if (text == NULL || ws_deserialize_duration(text, &duration)) {
			wsman_generate_op_fault(op, WSA_INVALID_MESSAGE_INFORMATION_HEADER,
					 WSMAN_DETAIL_OPERATION_TIMEOUT);
			return 0;
		}
		if (duration <= 0) {
			wsman_generate_op_fault(op, WSMAN_TIMED_OUT, 0);
			return 0;
		}
		op->expires = duration;
		// Not supported now
		if (ws_xml_find_attr_bool(child, nsUri, SOAP_MUST_UNDERSTAND)) {
			/*
			wsman_generate_op_fault(op, WSA_INVALID_MESSAGE_INFORMATION_HEADER,
						0);
		} else {*/
			wsman_generate_op_fault(op, WSMAN_UNSUPPORTED_FEATURE,
						WSMAN_DETAIL_OPERATION_TIMEOUT);
			return 0;
		}
	}
	return 1;
}


static WsXmlNodeH
validate_mustunderstand_headers(op_t * op)
{
	WsXmlNodeH      child = NULL;
	WsXmlNodeH      header;
	int             i;
	char           *nsUri;

	header = wsman_get_soap_header_element(op->dispatch->fw,
					 op->in_doc, NULL, NULL);
	nsUri = ws_xml_get_node_name_ns(header);

	for (i = 0; (child = ws_xml_get_child(header, i, NULL, NULL)) != NULL; i++) {
		if (ws_xml_find_attr_bool(child, nsUri, SOAP_MUST_UNDERSTAND)) {
			lnode_t        *node = list_first(op->processed_headers);
			while (node != NULL) {
				if (node->list_data == node)
					break;
				node = list_next(op->processed_headers, node);
			}
			if (node == NULL) {
				if (!is_wk_header(child)) {
					break;
				}
			}
		}
	}

	if (child != NULL) {
		debug("Mustunderstand Fault: %s", ws_xml_get_node_text(child));
	}
	return child;
}



/**
 * Check for duplicate Message ID
 * @param op operation
 * @return status
 */
static int
wsman_check_unsupported_features(op_t * op)
{
	WsXmlNodeH      enumurate;
	WsXmlNodeH      header = wsman_get_soap_header_element(op->dispatch->fw,
						    op->in_doc, NULL, NULL);
	WsXmlNodeH      body = ws_xml_get_soap_body(op->in_doc);
	int             retVal = 0;
	SoapH           soap;
	WsXmlNodeH      n, m;
	soap = op->dispatch->fw;

	n = ws_xml_get_child(header, 0, XML_NS_ADDRESSING, WSA_FAULT_TO);
	if (n != NULL) {
		retVal = 1;
		wsman_generate_op_fault(op, WSMAN_UNSUPPORTED_FEATURE,
				WSMAN_DETAIL_ADDRESSING_MODE);
	}
	n = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_LOCALE);
	if (n != NULL) {
		debug("Locale header found");
		char *mu = ws_xml_find_attr_value(n, XML_NS_SOAP_1_2 , SOAP_MUST_UNDERSTAND);
		if (mu != NULL && strcmp(mu, "true") == 0 ) {
			retVal = 1;
			wsman_generate_op_fault(op, WSMAN_UNSUPPORTED_FEATURE,
					WSMAN_DETAIL_LOCALE);
		}
	}
	n = ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_FRAGMENT_TRANSFER);
	if (n != NULL) {
		debug("FragmentTransfer header found");
		char *mu = ws_xml_find_attr_value(n, XML_NS_SOAP_1_2 , SOAP_MUST_UNDERSTAND);
		if (mu != NULL && strcmp(mu, "true") == 0 ) {
			retVal = 1;
			wsman_generate_op_fault(op, WSMAN_UNSUPPORTED_FEATURE,
					WSMAN_DETAIL_FRAGMENT_LEVEL_ACCESS);
		}
	}

	enumurate = ws_xml_get_child(body, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE);
	n = ws_xml_get_child(enumurate, 0, XML_NS_ENUMERATION, WSENUM_END_TO);
	if (n != NULL) {
		retVal = 1;
		wsman_generate_op_fault(op, WSMAN_UNSUPPORTED_FEATURE, 
				WSMAN_DETAIL_ADDRESSING_MODE);
	}	
	n = ws_xml_get_child(enumurate, 0, XML_NS_ENUMERATION, WSENUM_FILTER);
	m = ws_xml_get_child(enumurate, 0, XML_NS_WS_MAN , WSM_FILTER);
	if (n != NULL && m!= NULL) {
		retVal = 1;
		wsman_generate_op_fault(op, WSEN_CANNOT_PROCESS_FILTER, 0);
	}	

	return retVal;
}

/**
 * Check for duplicate Message ID
 * @param op operation
 * @return status
 */
static int
wsman_is_duplicate_message_id(op_t * op)
{
	WsXmlNodeH header = wsman_get_soap_header_element(op->dispatch->fw,
					 op->in_doc, NULL, NULL);
	int             retVal = 0;
	SoapH           soap;
	WsXmlNodeH      msgIdNode;
	soap = op->dispatch->fw;

	msgIdNode = ws_xml_get_child(header, 0,
		      XML_NS_ADDRESSING, WSA_MESSAGE_ID);
	if (msgIdNode!= NULL) {
		lnode_t        *node;
		char           *msgId;

		msgId = ws_xml_get_node_text(msgIdNode);
		debug("Checking Message ID: %s", msgId);
		u_lock(soap);
		node = list_first(soap->processedMsgIdList);
		while (node != NULL) {
			if (!strcmp(msgId, (char *) node->list_data)) {
				debug("Duplicate Message ID: %s", msgId);
				retVal = 1;
				wsman_generate_op_fault(op,
							WSA_INVALID_MESSAGE_INFORMATION_HEADER,
							WSA_DETAIL_DUPLICATE_MESSAGE_ID );
				break;
			}
			node = list_next(soap->processedMsgIdList, node);
		}

		if (!retVal) {
			while (list_count(soap->processedMsgIdList) >=
			       PROCESSED_MSG_ID_MAX_SIZE) {
				node = list_del_first(soap->processedMsgIdList);
				u_free(node->list_data);
				u_free(node);
			}

			node = lnode_create(NULL);
			if (node) {
				node->list_data = u_str_clone(msgId);
				if (node->list_data == NULL) {
					u_free(node);
				} else {
					list_append(soap->processedMsgIdList, node);
				}
			}
		}
		u_unlock(soap);
	} else if (!wsman_is_identify_request(op->in_doc)) {
		wsman_generate_op_fault(op, WSA_INVALID_MESSAGE_INFORMATION_HEADER, 0 );
		debug("No MessageId found");
		return 1;
	}

	return retVal;
}

static int
process_filter_chain(op_t * op,
		     list_t * list)
{
	int             retVal = 0;
	callback_t     *filter = (callback_t *) list_first(list);
	while (!retVal && filter != NULL) {
		retVal = filter->proc((SoapOpH) op, filter->node.list_data);
		filter = (callback_t *) list_next(list, &filter->node);
	}
	return retVal;
}

/**
 * Process Filters
 * @param op SOAP operation
 * @param inbound Direction of message, 0 for outbound  and 1 for inbound.
 * @return 0 on success, 1 on error.
 **/
static int
process_filters(op_t * op,
				int inbound)
{
	int             retVal = 0;
	list_t         *list;

	debug("Processing Filters: %s", (!inbound) ? "outbound" : "inbound");
	if (!(op->dispatch->flags & SOAP_SKIP_DEF_FILTERS)) {
		list = inbound ? op->dispatch->fw->inboundFilterList :
			op->dispatch->fw->outboundFilterList;
		retVal = process_filter_chain(op, list);
	}
	if (retVal) {
		debug("process_filter_chain returned 1 for DEF FILTERS");
		return 1;
	}
	
	list = inbound ? op->dispatch->inboundFilterList :
			op->dispatch->outboundFilterList;
	retVal = process_filter_chain(op, list);
	if (retVal) {
		debug("process_filter_chain returned 1");
		return 1;
	}

	if (inbound) {
		WsXmlNodeH      notUnderstoodHeader;
		if (wsman_is_duplicate_message_id(op)) {
			debug("wsman_is_duplicate_message_id");
			return 1;
		}
		if (wsman_check_unsupported_features(op)) {
			debug("wsman_check_unsupported_features");
			return 1;
		}
		if ((notUnderstoodHeader = validate_mustunderstand_headers(op)) != 0) {
			wsman_generate_notunderstood_fault(op, notUnderstoodHeader);
			debug("validate_mustunderstand_headers");
			return 1;
		} 
		if (!validate_control_headers(op)) {
			debug("validate_control_headers");
			return 1;
		}
		if (check_for_duplicate_selectors(op)) {
			debug("check_for_duplicate_selectors");
			return 1;
		}
	}
	return 0;
}

static int
soap_add_disp_filter(SoapDispatchH disp,
		     SoapServiceCallback callbackProc,
		     void *callbackData,
		     int inbound)
{
	callback_t     *entry = NULL;
	if (disp) {
		list_t         *list = (!inbound) ?
		disp->outboundFilterList : disp->inboundFilterList;
		entry = make_callback_entry(callbackProc, callbackData, list);
	}
	return (entry == NULL);
}


int
soap_add_op_filter(SoapOpH op,
		   SoapServiceCallback proc,
		   void *data,
		   int inbound)
{
	if (op)
		return soap_add_disp_filter((SoapDispatchH) ((op_t *) op)->dispatch,
					    proc,
					    data,
					    inbound);
	return 1;
}


int
soap_add_filter(SoapH soap,
		SoapServiceCallback callbackProc,
		void *callbackData,
		int inbound)
{
	callback_t     *entry = NULL;
	if (soap) {
		list_t         *list = (!inbound) ?
		soap->outboundFilterList :
		soap->inboundFilterList;
		entry = make_callback_entry(callbackProc, callbackData, list);
	}
	return (entry == NULL);
}


int
outbound_control_header_filter(SoapOpH opHandle,
			       void *data)
{
	unsigned long   size = 0, envelope_size = 0;
	char           *buf = NULL;
	int             len;
	char *mu	= NULL;
	SoapH           soap = soap_get_op_soap(opHandle);
	WsXmlDocH       in_doc = soap_get_op_doc(opHandle, 1);
	WsXmlDocH       out_doc = soap_get_op_doc(opHandle, 0);
	WsXmlNodeH      inHeaders = wsman_get_soap_header_element(soap, in_doc, NULL, NULL);
	WsXmlNodeH	maxsize;

	if (!inHeaders) 
		return 0;

	maxsize = ws_xml_get_child(inHeaders, 0, XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE);
	mu = ws_xml_find_attr_value(maxsize, XML_NS_SOAP_1_2 , SOAP_MUST_UNDERSTAND);
	if (mu != NULL && strcmp(mu, "true") == 0 ) {
		size = ws_deserialize_uint32(NULL, inHeaders, 0,
				XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE);
		ws_xml_dump_memory_enc(out_doc, &buf, &len, "UTF-8");
		envelope_size = ws_xml_utf8_strlen(buf);
		if (envelope_size > size) {
			wsman_generate_op_fault((op_t *) opHandle, WSMAN_ENCODING_LIMIT,
					WSMAN_DETAIL_MAX_ENVELOPE_SIZE);
		}
		u_free(buf);
	}
	return 0;
}

int
outbound_addressing_filter(SoapOpH opHandle,
			   void *data)
{
	SoapH           soap = soap_get_op_soap(opHandle);
	WsXmlDocH       in_doc = soap_get_op_doc(opHandle, 1);
	WsXmlDocH       out_doc = soap_get_op_doc(opHandle, 0);

	WsXmlNodeH      outHeaders = wsman_get_soap_header_element(soap, out_doc, NULL, NULL);

	if (outHeaders) {
		if (ws_xml_get_child(outHeaders, 0,
				XML_NS_ADDRESSING, WSA_MESSAGE_ID) == NULL &&
				!wsman_is_identify_request(in_doc)) {
			char            uuidBuf[100];
			generate_uuid(uuidBuf, sizeof(uuidBuf), 0);
			ws_xml_add_child(outHeaders, XML_NS_ADDRESSING,
						 WSA_MESSAGE_ID, uuidBuf);
			debug("Adding message id: %s", uuidBuf);
		}
		if (in_doc != NULL) {
			WsXmlNodeH      inMsgIdNode;
			inMsgIdNode = wsman_get_soap_header_element(soap,
				in_doc, XML_NS_ADDRESSING, WSA_MESSAGE_ID);
			if (inMsgIdNode != NULL && !ws_xml_get_child(outHeaders, 0,
				       XML_NS_ADDRESSING, WSA_RELATES_TO)) {
				ws_xml_add_child(outHeaders, XML_NS_ADDRESSING,
					WSA_RELATES_TO,
					ws_xml_get_node_text(inMsgIdNode));
			}
		}
	}
	return 0;
}



/**
 * List all dispatcher interfaces
 * @param interfaces Dispatcher interfaces
 */
#ifdef DEBUG_VERBOSE
void 
wsman_dispatcher_list(list_t * interfaces)
{
	lnode_t        *node = list_first(interfaces);
	while (node) {
		WsDispatchInterfaceInfo *interface =
				(WsDispatchInterfaceInfo *) node->list_data;
		debug("Listing Dispatcher: interface->wsmanResourceUri: %s",
			 interface->wsmanResourceUri);
		node = list_next(interfaces, node);
	}
}
#else
#define wsman_dispatcher_list
#endif

static void
dispatcher_create_fault(SoapH soap,
			WsmanMessage * msg, WsXmlDocH in_doc)
{
	char           *buf = NULL;
	int             len;
	if (wsman_fault_occured(msg)) {
		wsman_generate_fault_buffer(
					    soap->cntx,
					    in_doc,
					    msg->status.fault_code,
					    msg->status.fault_detail_code,
					    msg->status.fault_msg,
					    &buf, &len);
		u_buf_set(msg->response, buf, len);
		u_free(buf);
		msg->http_code = wsman_find_httpcode_for_fault_code(msg->status.fault_code);
	}
}


int
process_inbound_operation(op_t * op,
			  WsmanMessage * msg)
{
	int             retVal = 1;
	char           *buf = NULL;
	int             len;

	msg->http_code = WSMAN_STATUS_OK;
	op->out_doc = NULL;
	if (op->dispatch->serviceCallback == NULL) {
		wsman_set_fault(msg, WSA_ACTION_NOT_SUPPORTED,
				    OWSMAN_NO_DETAILS, NULL);
		debug("op service callback is null");
		goto GENERATE_FAULT;
	}

	if (process_filters(op, 1)) {
		if (op->out_doc == NULL) {
			error("doc is null");
			wsman_set_fault(msg, WSMAN_INTERNAL_ERROR,
				    OWSMAN_NO_DETAILS, NULL);
			goto GENERATE_FAULT;
		}
		if (wsman_is_fault_envelope(op->out_doc)) {
			msg->http_code = wsman_find_httpcode_for_value(op->out_doc);
		} else {
			error("not fault envelope");
		}

		ws_xml_dump_memory_enc(op->out_doc, &buf, &len, "UTF-8");
		u_buf_set(msg->response, buf, len);
		ws_xml_destroy_doc(op->out_doc);
		op->out_doc = NULL;
		u_free(buf);
		return 1;
	}

	retVal = op->dispatch->serviceCallback((SoapOpH) op, op->dispatch->serviceData);
	if (op->out_doc == NULL) {
		// XXX (correct fault?)
		wsman_set_fault(msg, WSA_DESTINATION_UNREACHABLE,
				    WSMAN_DETAIL_INVALID_RESOURCEURI, NULL);
		error("output doc is null");
		goto GENERATE_FAULT;
	}

	process_filters(op, 0);
	if (op->out_doc == NULL) {
		error("doc is null");
		wsman_set_fault(msg, WSMAN_INTERNAL_ERROR,
				    OWSMAN_NO_DETAILS, NULL);
		goto GENERATE_FAULT;
	}
	if (wsman_is_fault_envelope(op->out_doc)) {
		msg->http_code = wsman_find_httpcode_for_value(op->out_doc);
	}

	ws_xml_dump_memory_enc(op->out_doc, &buf, &len, "UTF-8");
	u_buf_set(msg->response, buf, len);
	ws_xml_destroy_doc(op->out_doc);
	op->out_doc = NULL;
	u_free(buf);
	return 0;

GENERATE_FAULT:
	// dispatcher_create_fault() will be called by caller
	return retVal;
}


static SoapDispatchH
get_dispatch_entry(SoapH soap, 
		   WsXmlDocH doc)
{
	SoapDispatchH dispatch = NULL;
	if (soap->dispatcherProc) {
		dispatch = soap->dispatcherProc(soap->cntx,
						 soap->dispatcherData, doc);
	}
	if (dispatch == NULL) {
		error("Dispatcher Error");
	} else {
		dispatch->usageCount++;
	}
	return dispatch;
}

void
dispatch_inbound_call(SoapH soap,
		      WsmanMessage * msg)
{
	op_t           *op = NULL;
	WsXmlDocH       in_doc = wsman_build_inbound_envelope(soap, msg);
	SoapDispatchH dispatch = NULL;
	debug("Inbound call...");

	if (wsman_fault_occured(msg)) {
		debug("doc == NULL || wsman_fault_occured(msg)");
		goto DONE;
	}
	dispatch = get_dispatch_entry(soap, in_doc);

	if (dispatch == NULL) {
		wsman_set_fault(msg, WSA_DESTINATION_UNREACHABLE,
				    WSMAN_DETAIL_INVALID_RESOURCEURI, NULL);
		debug("dispatch == NULL");
		goto DONE;
	}
	op = create_op_entry(soap, dispatch, msg);
	if (op == NULL) {
		wsman_set_fault(msg, WSA_DESTINATION_UNREACHABLE,
				    WSMAN_DETAIL_INVALID_RESOURCEURI, NULL);
		destroy_dispatch_entry(dispatch);
		debug("dispatch == NULL");
		goto DONE;
	}
	op->in_doc = in_doc;
	process_inbound_operation(op, msg);
DONE:
	dispatcher_create_fault(soap, msg, in_doc);
	destroy_op_entry(op);
	ws_xml_destroy_doc(in_doc);
	debug("Inbound call completed");
	return;
}


static char    *
wsman_dispatcher_match_ns(WsDispatchInterfaceInfo * r,
			  char *uri)
{
	char           *ns = NULL;
	if (r->namespaces == NULL) {
		return NULL;
	}
	if (uri) {
		lnode_t *node = list_first(r->namespaces);
		while (node) {
			WsSupportedNamespaces *sns =
				(WsSupportedNamespaces *) node->list_data;
			if (sns->ns != NULL && strstr(uri, sns->ns)) {
				ns = u_strdup(sns->ns);
				break;
			}
			node = list_next(r->namespaces, node);
		}
	}
	return ns;
}


WsEndPointRelease
wsman_get_release_endpoint(WsContextH cntx, WsXmlDocH doc)
{
	WsManDispatcherInfo *dispInfo =
		(WsManDispatcherInfo *)cntx->soap->dispatcherData;
	char           *uri = NULL;
	lnode_t        *node = list_first((list_t *)dispInfo->interfaces);
	WsDispatchInterfaceInfo *r = NULL;	
	WsDispatchEndPointInfo *ep = NULL;
	uri = wsman_get_resource_uri(cntx, doc);
	char           *ns = NULL;
	char           *ptr = ENUM_ACTION_RELEASE;
	int i;

	while (node != NULL) {
		WsDispatchInterfaceInfo *ifc =
			(WsDispatchInterfaceInfo *) node->list_data;
		if (ifc->wsmanResourceUri == NULL &&
				(ns = wsman_dispatcher_match_ns(ifc, uri))) {
			r = ifc;
			break;
		}
		if (ifc->wsmanResourceUri &&
				!strcmp(uri, ifc->wsmanResourceUri)) {
			r = ifc;
			break;
		}
		node = list_next((list_t *) dispInfo->interfaces, node);
	}
	if (r == NULL) {
		u_free(ns);
		return NULL;
	}
	/*
        * See if the action is part of the namespace which means that
        * we are dealing with a custom action
        */
	if (ns != NULL) {
		size_t             len = strlen(ns);
		if (!strncmp(ptr, ns, len) && ptr[len] == '/') {
			ptr += len + 1;
		}
	}
	for (i = 0; r->endPoints[i].serviceEndPoint != NULL; i++) {
		if (r->endPoints[i].inAction != NULL &&
		     !strcmp(ptr, r->endPoints[i].inAction)) {
			ep = &r->endPoints[i];
			break;
		}
	}
	u_free(ns);

	if (ep == NULL) {
		debug("no ep");
		return NULL;
	}
	debug("Release endpoint: %p", ep->serviceEndPoint);
	return (WsEndPointRelease)ep->serviceEndPoint;
}





SoapDispatchH
wsman_dispatcher(WsContextH cntx,
		 void *data,
		 WsXmlDocH doc)
{
	SoapDispatchH   disp = NULL;
	char           *uri = NULL, *action;
	WsManDispatcherInfo *dispInfo = (WsManDispatcherInfo *) data;
	WsDispatchEndPointInfo *ep = NULL;
	WsDispatchEndPointInfo *ep_custom = NULL;
	int             i, resUriMatch = 0;
	char           *ns = NULL;

	WsDispatchInterfaceInfo *r = NULL;
	lnode_t        *node = list_first((list_t *) dispInfo->interfaces);

	if (doc == NULL) {
		error("doc is null");
		u_free(data);
		goto cleanup;
	}
	uri = wsman_get_resource_uri(cntx, doc);
	action = wsman_get_action(cntx, doc);
	if ((!uri || !action) && !wsman_is_identify_request(doc)) {
		goto cleanup;
	}

	while (node != NULL) {
		WsDispatchInterfaceInfo *ifc = (WsDispatchInterfaceInfo *) node->list_data;
		if (wsman_is_identify_request(doc)) {
			if ((ns = wsman_dispatcher_match_ns(ifc, XML_NS_WSMAN_ID))) {
				r = ifc;
				resUriMatch = 1;
				break;
			}
			debug("ns did not match");
		}
		 /*
	          * If Resource URI is null then most likely we are dealing
		  * with  a generic plugin supporting a namespace with
		  *  multiple Resource URIs (e.g. CIM)
	          **/
		else if (ifc->wsmanResourceUri == NULL &&
				(ns = wsman_dispatcher_match_ns(ifc, uri))) {
			r = ifc;
			resUriMatch = 1;
			break;
		} else if (ifc->wsmanResourceUri &&
				!strcmp(uri, ifc->wsmanResourceUri)) {
			r = ifc;
			resUriMatch = 1;
			break;
		}
		node = list_next((list_t *) dispInfo->interfaces, node);
	}

	if (wsman_is_identify_request(doc) && r != NULL) {
		ep = &r->endPoints[0];
	} else if (r != NULL) {
		char           *ptr = action;
		/*
	         * See if the action is part of the namespace which means that
	         * we are dealing with a custom action
	         */
		if (ns != NULL) {
			size_t             len = strlen(ns);
			if (!strncmp(action, ns, len) && action[len] == '/')
				ptr = &action[len + 1];
		}
		for (i = 0; r->endPoints[i].serviceEndPoint != NULL; i++) {
			if (r->endPoints[i].inAction != NULL &&
				    !strcmp(ptr, r->endPoints[i].inAction)) {
				ep = &r->endPoints[i];
				break;
			} else if (r->endPoints[i].inAction == NULL) {
				/*
				 * Just store it for later
				 * in case no match is found for above condition 
				 */
				ep_custom = &r->endPoints[i];
			}
		}
	}
	ws_remove_context_val(cntx, WSM_RESOURCE_URI);

	if (ep != NULL) {
		for (i = 0; i < dispInfo->mapCount; i++) {
			if (dispInfo->map[i].ep == ep) {
				disp = dispInfo->map[i].disp;
				break;
			}
		}
	} else if (ep_custom != NULL) {
		for (i = 0; i < dispInfo->mapCount; i++) {
			if (dispInfo->map[i].ep == ep_custom) {
				disp = dispInfo->map[i].disp;
				break;
			}
		}
	}
cleanup:
	if (ns)
		u_free(ns);
	return disp;
}


/*
 * Create dispatch Entry
 *
 * @todo support for custom roles
 * @param fw Soap Framework Handle
 * @param inboundAction Inbound Action
 * @param outboundAction Outbound Action
 * @param role Role
 * @param proc Call back processor
 * @param data Callback Data
 * @param flags Flags
 * @return Dispatch Entry
 */
static SoapDispatchH
create_dispatch_entry(SoapH soap,
		      char *inboundAction,
		      char *outboundAction,
		      char *role,
		      SoapServiceCallback proc,
		      void *data,
		      unsigned long flags)
{

	SoapDispatchH entry = wsman_dispatch_entry_new();
	if (entry) {
		entry->fw = soap;
		entry->flags = flags;
		entry->inboundAction = u_str_clone(inboundAction);
		entry->outboundAction = u_str_clone(outboundAction);
		entry->serviceCallback = proc;
		entry->serviceData = data;
		entry->usageCount = 1;
		entry->inboundFilterList = list_create(LISTCOUNT_T_MAX);
		entry->outboundFilterList = list_create(LISTCOUNT_T_MAX);
	}
	return entry;
}



/*
 * Create Dispatch Entry
 * @param soap Soap handle
 * @param inboundAction Inbound Action
 * @param outboundAction Outbound Action (optional)
 * @param role Role (reserved, must be NULL)
 * @param callbackProc Callback processor
 * @param callbackData Callback data
 * @param flags Flags
 * @return Dispatch Handle
 */
SoapDispatchH
soap_create_dispatch(SoapH soap,
		     char *inboundAction,
		     char *outboundAction, //optional
		     char *role, //reserved, must be NULL
		     SoapServiceCallback callbackProc,
		     void *callbackData,
		     unsigned long flags)
{

	SoapDispatchH disp = NULL;
	debug("Creating dispatch");
	if (soap && role == NULL) {
		disp = create_dispatch_entry(soap, inboundAction, outboundAction,
				   role, callbackProc, callbackData, flags);
	}
	return disp;
}

/*
 * Start Dispatcher
 */
void 
soap_start_dispatch(SoapDispatchH disp)
{
	if (disp) {
		list_append(disp->fw->dispatchList,
			    &(disp)->node);
	}
}



SoapDispatchH
wsman_dispatch_entry_new(void) {
	return (SoapDispatchH)u_zalloc(sizeof (struct __dispatch_t));
}



/** @} */
