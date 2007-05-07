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


#ifndef WSMAN_DISPATCHER_H_
#define WSMAN_DISPATCHER_H_


#include "wsman-soap.h"

/**
 * @addtogroup Dispatcher
 *
 * @{
 */

struct __dispatch_t {
	lnode_t         node;
	int             usageCount;
	char           *inboundAction;
	char           *outboundAction;
	unsigned long   flags;
	SoapH           fw;
	SoapServiceCallback serviceCallback;
	void           *serviceData;
	list_t         *inboundFilterList;
	list_t         *outboundFilterList;
};

struct __op_t {
	SoapDispatchH dispatch;
	time_t          expires;
	unsigned long   submittedTicks;
	WsContextH      cntx;
	WsXmlDocH       in_doc;
	              //not deleted on destroy
        WsXmlDocH out_doc;
	              //not deleted on destroy
        WsmanMessage * data;
	list_t         *processed_headers;
};
typedef struct __op_t op_t;


int             soap_add_op_filter(SoapOpH op, SoapServiceCallback proc, void *data, int inbound);

int             outbound_addressing_filter(SoapOpH opHandle, void *data, void *opaqueData);
int             outbound_control_header_filter(SoapOpH opHandle, void *data, void *opaqueData);

int  			soap_add_filter(SoapH soap,	SoapServiceCallback callbackProc, void *callbackData,
				int inbound);


void		    wsman_generate_notunderstood_fault(op_t * op,
				   WsXmlNodeH notUnderstoodHeader);

char           *get_relates_to_message_id(SoapH soap, WsXmlDocH doc);

void            dispatch_inbound_call(SoapH soap, WsmanMessage * msg, void * opaqueData);
void            wsman_dispatcher_list(list_t * interfaces);

SoapDispatchH   wsman_dispatcher(WsContextH cntx, void *data, WsXmlDocH doc);
WsEndPointRelease wsman_get_release_endpoint(WsContextH cntx, WsXmlDocH doc);

void            destroy_op_entry(op_t * entry);

op_t*           create_op_entry(SoapH soap, SoapDispatchH dispatch,
				WsmanMessage * data);

int             unlink_response_entry(SoapH soap, op_t * entry);
op_t           *find_response_entry(SoapH soap, char *id);
void            destroy_dispatch_entry(SoapDispatchH entry);

void            add_response_entry(SoapH soap, op_t * op);

int             process_inbound_operation(op_t * op, WsmanMessage * msg, void * opaqueData);
void            wsman_create_identify_response(SoapH soap, WsmanMessage * msg);
void            wsman_generate_encoding_fault(op_t * op, WsmanFaultDetailType faultDetail);


void            soap_start_dispatch(SoapDispatchH disp);

SoapDispatchH   soap_create_dispatch(SoapH soap, char *inboundAction,
		     		char *outboundAction, //optional
		     		char *role, //reserved, must be NULL
		     		SoapServiceCallback callbackProc,
		     		void *callbackData,
		     		unsigned long flags);

SoapDispatchH  wsman_dispatch_entry_new(void);

/** @} */
#endif				/* WS_DISPATCHER_H_ */
