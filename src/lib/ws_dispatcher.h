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


#ifndef WS_DISPATCHER_H_
#define WS_DISPATCHER_H_




/**
 * @addtogroup Dispatcher 
 * 
 * @{
 */



struct __SOAP_CALLBACK_ENTRY
{
	DL_Node node; // dataBuf is passed to callback as data
	SoapServiceCallback proc;
};
typedef struct __SOAP_CALLBACK_ENTRY SOAP_CALLBACK_ENTRY;

struct __SOAP_DISPATCH_ENTRY
{
	DL_Node node;
	int usageCount;
	char* inboundAction;
	char* outboundAction;
	unsigned long flags;
	SOAP_FW* fw;
	SoapServiceCallback serviceCallback;
	void* serviceData;
	DL_List inboundFilterList;
	DL_List outboundFilterList;
};
typedef struct __SOAP_DISPATCH_ENTRY SOAP_DISPATCH_ENTRY;

struct __SOAP_OP_ENTRY
{
	SOAP_DISPATCH_ENTRY* dispatch;
	unsigned long timeoutTicks;
	unsigned long submittedTicks;
	WsContextH cntx;
	DL_List processedHeaders;
	WsXmlDocH inDoc;   // not deleted on destroy
	WsXmlDocH outDoc;  // not deleted on destroy
        WsmanMessage *data;
};
typedef struct __SOAP_OP_ENTRY SOAP_OP_ENTRY;

void wsman_generate_notunderstood_fault(
	SOAP_OP_ENTRY* op, 
	WsXmlNodeH notUnderstoodHeader
	);
char* get_relates_to_message_id(SOAP_FW* fw, WsXmlDocH doc);
 
void  dispatch_inbound_call(SOAP_FW *fw, WsmanMessage *msg);
void wsman_dispatcher_list( GList *interfaces );

SOAP_DISPATCH_ENTRY* create_dispatch_entry(SOAP_FW* fw,
        char* inboundAction, 
        char* outboundAction,
        char* role,
        SoapServiceCallback proc,
        void* data,
        unsigned long flags);


SOAP_CALLBACK_ENTRY* make_soap_callback_entry(SoapServiceCallback proc,
        void* data,
        DL_List* listToAdd);

int is_wk_header(WsXmlNodeH header);
WsXmlNodeH get_soap_header_element(SOAP_FW *fw, WsXmlDocH doc, char *nsUri, char *name);
char *get_soap_header_value(SOAP_FW *fw, WsXmlDocH doc, char *nsUri, char *name);
int ws_is_duplicate_message_id(SOAP_FW *fw, WsXmlDocH doc);
int is_valid_envelope(SOAP_FW *fw, WsXmlDocH doc);


// Temp
WsXmlDocH build_inbound_envelope(SOAP_FW* fw,  WsmanMessage *msg );
SOAP_DISPATCH_ENTRY* do_get_dispatch_entry(SOAP_FW* fw, WsXmlDocH doc);
SOAP_DISPATCH_ENTRY* get_dispatch_entry(SOAP_FW* fw, WsXmlDocH doc);
SoapDispatchH wsman_dispatcher(WsContextH cntx, void* data, WsXmlDocH doc);

WsXmlDocH build_soap_fault(SOAP_FW *fw, char *soapNsUri, char *faultNsUri, char *code, char *subCode, char *reason, char *detail);
void build_soap_version_fault(SOAP_FW *fw);

void destroy_op_entry(SOAP_OP_ENTRY* entry);

SOAP_OP_ENTRY* create_op_entry(SOAP_FW* fw,
        SOAP_DISPATCH_ENTRY* dispatch,
        WsmanMessage *data,
        unsigned long timeout);

int unlink_response_entry(SOAP_FW* fw, SOAP_OP_ENTRY* entry);
SOAP_OP_ENTRY* find_response_entry(SOAP_FW* fw, char* id);
void destroy_dispatch_entry(SOAP_DISPATCH_ENTRY* entry);  
     
void add_response_entry(SOAP_FW* fw, SOAP_OP_ENTRY* op);

int process_filter_chain(SOAP_OP_ENTRY* op, DL_List* list);

WsXmlNodeH validate_mustunderstand_headers(SOAP_OP_ENTRY* op);
int process_filters(SOAP_OP_ENTRY* op, int inbound);
WsmanFaultCodeType ws_is_valid_envelope(SOAP_FW* fw, WsXmlDocH doc);

int process_inbound_operation(SOAP_OP_ENTRY* op, WsmanMessage *msg);
int wsman_is_identify_request(WsXmlDocH doc);
void wsman_create_identify_response(SOAP_FW *fw, WsmanMessage *msg);
void wsman_generate_encoding_fault( SOAP_OP_ENTRY* op, WsmanFaultDetailType faultDetail);
int validate_control_headers(SOAP_OP_ENTRY* op);


/** @} */
#endif /*WS_DISPATCHER_H_*/
