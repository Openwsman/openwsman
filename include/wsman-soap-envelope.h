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

#include "wsman-types.h"
#include "wsman-soap.h"
#include "wsman-soap-message.h"

#define ENFORCE_MUST_UNDERSTAND	"EnforceMustUnderstand"

/* special hash key to denote method args (where array elements have identical keys) */
#define METHOD_ARGS_KEY "method_args"

int wsman_is_valid_envelope(WsmanMessage * msg, WsXmlDocH doc);

char *wsman_get_soap_header_value( WsXmlDocH doc, const char *nsUri,
				  			const char *name);

WsXmlNodeH wsman_get_soap_header_element(WsXmlDocH doc, const char *nsUri,
					 const char *name);

WsXmlDocH wsman_build_soap_fault( const char *soapNsUri,
				 const char *faultNsUri, const char *code,
				 const char *subCode, const char *reason,
				 const char *detail);


WsXmlDocH wsman_create_response_envelope(WsXmlDocH rqstDoc, const char *action);

WsXmlDocH wsman_build_inbound_envelope( WsmanMessage * msg);

WsXmlDocH wsman_create_fault_envelope(
				      WsXmlDocH rqstDoc,
				      const char *code,
				      const char *subCodeNs,
				      const char *subCode,
				      const char *fault_action,
				      const char *lang,
				      const char *reason, const char *faultDetail);

char *wsman_get_class_name(WsContextH cntx);

char *wsman_get_method_name(WsContextH cntx);

hash_t *wsman_get_method_args(WsContextH cntx, const char *resource_uri);

int wsman_get_max_elements(WsContextH cntx, WsXmlDocH doc);

unsigned long wsman_get_max_envelope_size(WsContextH cntx, WsXmlDocH doc);

char *wsman_get_fragment_string(WsContextH cntx, WsXmlDocH doc);

void wsman_get_fragment_type(char *fragstr, int *fragment_flag, char **element,
	int *index);

void wsman_set_estimated_total(WsXmlDocH in_doc,
			       WsXmlDocH out_doc,
			       WsEnumerateInfo * enumInfo);

char *wsman_get_selector(WsContextH cntx, WsXmlDocH doc, const char *name,
			 const int index);

hash_t *wsman_get_selectors_from_epr(WsContextH cntx, WsXmlNodeH epr_node);

hash_t *wsman_get_selector_list(WsContextH cntx, WsXmlDocH doc);

hash_t *wsman_get_selector_list_from_filter(WsContextH cntx,
					    WsXmlDocH doc);

void wsman_add_selector(WsXmlNodeH baseNode, const char *name, const char *val);

char *wsman_get_action(WsContextH cntx, WsXmlDocH doc);

char *wsman_get_resource_uri(WsContextH cntx, WsXmlDocH doc);

int wsman_is_fault_envelope(WsXmlDocH doc);

void wsman_set_fault(WsmanMessage * msg,
		     WsmanFaultCodeType fault_code,
		     WsmanFaultDetailType fault_detail_code,
		     const char *details);

int wsman_is_identify_request(WsXmlDocH doc);
int wsman_check_identify(WsmanMessage * msg);

int wsman_is_event_related_request(WsXmlDocH doc);

int wsman_is_valid_xml_envelope(WsXmlDocH doc);

void wsman_add_namespace_as_selector(WsXmlDocH doc, const char *_namespace);

void wsman_add_fragement_for_header(WsXmlDocH indoc, WsXmlDocH outdoc);

char *wsman_get_option_set(WsContextH cntx, WsXmlDocH doc, const char *op);

int wsman_parse_enum_request(WsContextH cntx, WsEnumerateInfo * enumInfo);

int wsman_parse_event_request(WsXmlDocH doc, WsSubscribeInfo * subsInfo, WsmanFaultCodeType *faultcode,
	WsmanFaultDetailType *detailcode);

int wsman_parse_credentials(WsXmlDocH doc, WsSubscribeInfo * subsInfo, WsmanFaultCodeType *faultcode,
	WsmanFaultDetailType *detailcode);

void wsman_set_expiretime(WsXmlNodeH  node, unsigned long * expire, WsmanFaultCodeType *fault_code);

int time_expired(unsigned long lt);

WsXmlDocH wsman_create_doc(const char *rootname);

void wsman_destroy_doc(WsXmlDocH doc);

#endif
