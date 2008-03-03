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
	lnode_t node;
	int usageCount;
	char *inboundAction;
	char *outboundAction;
	unsigned long flags;
	SoapH soap;
	SoapServiceCallback serviceCallback;
	void *serviceData;
	list_t *inboundFilterList;
	list_t *outboundFilterList;
};

struct __op_t {
	SoapDispatchH dispatch;
	time_t expires;
	unsigned long maxsize; //max envelope size
	//unsigned long submittedTicks;
	WsContextH cntx;
	WsXmlDocH in_doc;
	//not deleted on destroy
	WsXmlDocH out_doc;
	//not deleted on destroy
	WsmanMessage *data;
	list_t *processed_headers;
};
typedef struct __op_t op_t;


void dispatch_inbound_call(SoapH soap, WsmanMessage * msg,
			   void *opaqueData);

SoapDispatchH wsman_dispatcher(WsContextH cntx, void *data, WsXmlDocH doc);

void destroy_op_entry(op_t * entry);

op_t *create_op_entry(SoapH soap, SoapDispatchH dispatch,
		      WsmanMessage * data);

int unlink_response_entry(SoapH soap, op_t * entry);

void destroy_dispatch_entry(SoapDispatchH entry);

WsEndPointRelease wsman_get_release_endpoint(WsContextH cntx,
					     WsXmlDocH doc);

void wsman_dispatch_start(SoapDispatchH disp);

SoapDispatchH wsman_dispatch_create(SoapH soap, char *inboundAction, char *outboundAction,
				   char *role,	//reserved, must be NULL
				   SoapServiceCallback callbackProc,
				   void *callbackData,
				   unsigned long flags);

SoapDispatchH wsman_dispatch_entry_new(void);

/** @} */
#endif				/* WS_DISPATCHER_H_ */
