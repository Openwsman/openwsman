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
 */

#ifndef WSMAN_SOAP_ENVELOPE_H_
#define WSMAN_SOAP_ENVELOPE_H_

#include "wsman-soap-api.h"

#define ENFORCE_MUST_UNDERSTAND	"EnforceMustUnderstand"

void wsman_is_valid_envelope(WsmanMessage *msg, WsXmlDocH doc);

int wsman_is_duplicate_message_id (SoapH soap, WsXmlDocH doc);

char* get_soap_header_value(SoapH soap, WsXmlDocH doc, char* nsUri, char* name);

WsXmlNodeH get_soap_header_element(SoapH soap,
        WsXmlDocH doc, char* nsUri, char* name);

WsXmlDocH build_soap_fault(SoapH soap, char *soapNsUri, char *faultNsUri, 
        char *code, char *subCode, char *reason, char *detail);

void build_soap_version_fault(SoapH soap);

WsXmlDocH ws_create_response_envelope(WsContextH cntx, 
        WsXmlDocH rqstDoc, 
        char* action);

WsXmlDocH wsman_build_inbound_envelope(SoapH soap, WsmanMessage *msg);

WsXmlDocH wsman_create_fault(WsContextH cntx, WsXmlDocH rqstDoc, char *code,
        char *subCodeNs, char *subCode, char *lang, 
        char *reason, void (*addDetailProc)(WsXmlNodeH, void *), void *addDetailProcData);


WsXmlDocH wsman_create_fault_envelope(WsContextH cntx,
            WsXmlDocH rqstDoc,
            char* code,
            char* subCodeNs,
            char* subCode,
            char* lang,
            char* reason,
            char* faultDetail);

char* wsman_get_class_name ( WsContextH cntx );

char* wsman_get_method_name ( WsContextH cntx ); 

hash_t* wsman_get_method_args ( WsContextH cntx, char *resource_uri );            

int wsen_get_max_elements(WsContextH cntx, WsXmlDocH doc);

void wsman_set_estimated_total(WsXmlDocH in_doc, WsXmlDocH out_doc, WsEnumerateInfo *enumInfo);

int wsman_is_optimization(WsContextH cntx, WsXmlDocH doc);

char * wsman_get_enum_mode(WsContextH cntx, WsXmlDocH doc);

void wsman_set_enum_mode(char *enum_mode, WsEnumerateInfo *enumInfo);

char* wsman_get_selector(WsContextH cntx, WsXmlDocH doc, char* name, int index);

hash_t * wsman_get_selector_list(WsContextH cntx, WsXmlDocH doc);
        
void wsman_add_selector( WsXmlNodeH baseNode, char* name, char* val);

char* ws_addressing_get_action(WsContextH cntx, WsXmlDocH doc);

char* wsman_get_system_uri(WsContextH cntx, WsXmlDocH doc);

char* wsman_get_resource_uri(WsContextH cntx, WsXmlDocH doc);

int wsman_is_fault_envelope(WsXmlDocH doc);

void wsman_set_fault(WsmanMessage *msg, 
            WsmanFaultCodeType fault_code, 
            WsmanFaultDetailType fault_detail_code,
            const char *details);

int wsman_is_identify_request(WsXmlDocH doc);

void 
wsman_set_polymorph_mode( WsContextH cntx,
                          WsXmlDocH doc,
                          WsEnumerateInfo *enumInfo);

#endif
