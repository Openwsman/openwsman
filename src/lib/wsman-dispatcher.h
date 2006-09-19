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




/**
 * @addtogroup Dispatcher 
 * 
 * @{
 */

struct __dispatch_t
{
	lnode_t node;
	int usageCount;
	char* inboundAction;
	char* outboundAction;
	unsigned long flags;
	env_t* fw;
	SoapServiceCallback serviceCallback;
	void* serviceData;
	list_t *inboundFilterList;
	list_t *outboundFilterList;
};
typedef struct __dispatch_t dispatch_t;

struct __op_t
{
	dispatch_t* dispatch;
	unsigned long timeoutTicks;
	unsigned long submittedTicks;
	WsContextH cntx;
	WsXmlDocH in_doc;   // not deleted on destroy
	WsXmlDocH out_doc;  // not deleted on destroy
        WsmanMessage *data;
};
typedef struct __op_t op_t;


void 
wsman_generate_notunderstood_fault( op_t* op, 
                                    WsXmlNodeH notUnderstoodHeader);

char* get_relates_to_message_id(env_t* fw, WsXmlDocH doc);
 
void  dispatch_inbound_call(env_t *fw, WsmanMessage *msg);
void wsman_dispatcher_list( list_t *interfaces );


dispatch_t* create_dispatch_entry(env_t* fw,
        char* inboundAction, 
        char* outboundAction,
        char* role,
        SoapServiceCallback proc,
        void* data,
        unsigned long flags);



int is_wk_header(WsXmlNodeH header);

dispatch_t* get_dispatch_entry(env_t* fw, WsXmlDocH doc);
SoapDispatchH wsman_dispatcher(WsContextH cntx, void* data, WsXmlDocH doc);


void destroy_op_entry(op_t* entry);

op_t* create_op_entry(env_t* fw,
        dispatch_t* dispatch,
        WsmanMessage *data,
        unsigned long timeout);

int unlink_response_entry(env_t* fw, op_t* entry);
op_t* find_response_entry(env_t* fw, char* id);
void destroy_dispatch_entry(dispatch_t* entry);  
     
void add_response_entry(env_t* fw, op_t* op);

int process_filter_chain(op_t* op, list_t* list);

WsXmlNodeH validate_mustunderstand_headers(op_t* op);
int process_filters(op_t* op, int inbound);

int process_inbound_operation(op_t* op, WsmanMessage *msg);
void wsman_create_identify_response(env_t *fw, WsmanMessage *msg);
void wsman_generate_encoding_fault( op_t* op, WsmanFaultDetailType faultDetail);
int validate_control_headers(op_t* op);

void soap_start_dispatch(SoapDispatchH disp);
SoapDispatchH soap_create_dispatch(SoapH soap, 
        char* inboundAction,
        char* outboundAction, // optional
        char* role, // reserved, must be NULL
        SoapServiceCallback callbackProc,
        void* callbackData,
        unsigned long flags);
        

dispatch_t* wsman_dispatch_entry_new(void);


/** @} */
#endif /*WS_DISPATCHER_H_*/
