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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <u/libu.h>
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-dispatcher.h"

#include "wsman-xml.h"
#include "wsman-xml-serializer.h" 
#include "wsman-soap-message.h"
#include "wsman-faults.h"


/*

void
wsman_assertion_failed(const char * const fileName,
                        int          const lineNumber) {

    fprintf(stderr, "%s:%d: assertion failed\n", fileName, lineNumber);
}
*/

static void 
add_details_proc(WsXmlNodeH fault,
                 void* data) 
{
    if (data == NULL)
        return;
    char* soapNs = ws_xml_get_node_name_ns(fault);
    WsXmlNodeH node = ws_xml_add_child(fault, soapNs, SOAP_DETAIL, NULL);
    node = ws_xml_add_child_format(node, XML_NS_WS_MAN, SOAP_FAULT_DETAIL, "%s/%s", XML_NS_WSMAN_FAULT_DETAIL, (char *)data);
    return;
}




void 
wsman_set_fault( WsmanMessage *msg, 
                 WsmanFaultCodeType fault_code, 
                 WsmanFaultDetailType fault_detail_code, 
                 const char *details) 
{
    msg->status.fault_code = fault_code;
    msg->status.fault_detail_code = fault_detail_code;
    if (details) msg->status.fault_msg = strdup(details);
    return;
}

static char *
get_fault_details(WsmanFaultDetailType fault_detail_code)
{
    char *descr;
    switch (fault_detail_code) 
    {
    case WSMAN_DETAIL_INVALID_RESOURCEURI:
        descr = 
            "The WS-Management service cannot process the request." \
            " The specified resource URI is missing or in an incorrect format.";
        break;
    case WSMAN_DETAIL_MAX_ENVELOPE_SIZE:
        descr 	= "The WS-Management service cannot process the request because" \
                   "the envelope size in the request is too small.";
                      
        break;
    case WSMAN_DETAIL_MAX_ENVELOPE_SIZE_EXCEEDED:
        descr 	= "The response that the WS-Management service computed exceeds" \
                  "the maximum envelope size in the request.";
        break;
    case OWSMAN_DETAIL_ENDPOINT_ERROR:
        descr = "Endpoint Failure";
        break;
    default:
        descr = NULL;
        break;
    }
    return descr;
}



int 
wsman_is_fault_envelope( WsXmlDocH doc ) 
{
    WsXmlNodeH node = ws_xml_get_child(ws_xml_get_soap_body(doc),  
            0 , XML_NS_SOAP_1_2 , SOAP_FAULT);
    if ( node != NULL )
        return 1;
    else
        return 0;
}


WsXmlDocH 
wsman_generate_fault( WsContextH cntx, 
                      WsXmlDocH in_doc, 
                      WsmanFaultCodeType faultCode, 
                      WsmanFaultDetailType faultDetail,
                      char *fault_msg)
{
    char *code = FAULT_SENDER_CODE;
    char *subCodeNs;
    char *subCode;
    char *reason;
    char *detail = NULL;

    debug( "Fault Code: %d", faultCode);
    switch (faultCode) 
    {
    case WSA_ENDPOINT_UNAVAILABLE:
        subCodeNs 	= XML_NS_ADDRESSING;
        subCode		= "EndpointUnavailable";
        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Endpoint unavailable.";
        break;
    case WSA_ACTION_NOT_SUPPORTED:
        subCodeNs 	= XML_NS_ADDRESSING;
        subCode		= "ActionNotSupported";
        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Action not supported.";

        break;
    case WSMAN_INVALID_SELECTORS:
        subCodeNs 	= XML_NS_WS_MAN;
        subCode		= "InvalidSelectors";
        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 	= "The WS-Management service cannot process the request " \
                "because the selectors for the resource are not valid.";

        break;

    case SOAP_FAULT_MUSTUNDERSTAND:
        subCodeNs 	= XML_NS_SOAP_1_2;
        subCode		= "MustUnderstand"; 		 			
        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Header not understood";
        break; 				 		 		 		
    case WSMAN_UNKNOWN:
    case WSMAN_INTERNAL_ERROR:
        subCodeNs 	= XML_NS_WS_MAN;
        subCode		= "InternalError";
        code			= FAULT_RECEIVER_CODE;

        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Internal Error."; 		 			
        break;
    case WSA_INVALID_MESSAGE_INFORMATION_HEADER:
        subCodeNs 	= XML_NS_ADDRESSING;
        subCode 	  	= "InvalidMessageInformationHeader";

        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Invalid Header"; 			 		
        break;
    case WSEN_INVALID_ENUMERATION_CONTEXT:
        subCodeNs 	= XML_NS_ENUMERATION;
        subCode 	  	= "InvalidEnumerationContext";
        code			= FAULT_RECEIVER_CODE;
        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "An invalid enumeration context was supplied with the message"; 			 		
        break; 		
    case WSMAN_ENCODING_LIMIT:
        subCodeNs 	= XML_NS_WS_MAN;
        subCode 	  	= "EncodingLimit";

        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 	= "Encoding Limit, No details..";

        break; 		
    case WSA_DESTINATION_UNREACHABLE:
        detail 		= "InvalidResourceURI";
        subCodeNs 	= XML_NS_ADDRESSING;
        subCode 	  	= "DestinationUnreachable";

        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Destination Unreachable";

        break; 		
    default:
        subCodeNs = NULL;
        subCode = NULL;
        reason = NULL;
        break;

    }; 
    if (fault_msg!= NULL ) {
        reason = fault_msg;
    }
    WsXmlDocH fault = ws_xml_create_fault(cntx, in_doc,
            code, subCodeNs, subCode, NULL, reason, add_details_proc, detail);	                    
    return fault;                        
}




void
wsman_generate_fault_buffer ( WsContextH cntx, 
                              WsXmlDocH in_doc, 
                              WsmanFaultCodeType faultCode, 
                              WsmanFaultDetailType faultDetail, 
                              char * fault_msg, 
                              char **buf,  
                              int* len)
{	
    debug( "Fault Code: %d", faultCode);
    WsXmlDocH doc = wsman_generate_fault(cntx, in_doc, faultCode, faultDetail, fault_msg);   
    ws_xml_dump_memory_enc(doc, buf, len, NULL);	
    ws_xml_destroy_doc(doc);
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
    op->out_doc = wsman_generate_fault(op->cntx,
            op->in_doc,
            SOAP_FAULT_MUSTUNDERSTAND,
            SOAP_DETAIL_HEADER_NOT_UNDERSTOOD,
            NULL
            );


    header = ws_xml_get_soap_header(op->out_doc);

    child = ws_xml_add_child(header, XML_NS_SOAP_1_2, "NotUnderstood", NULL);
    ws_xml_add_qname_attr(child, NULL, "qname", ws_xml_get_node_name_ns(notUnderstoodHeader),
            ws_xml_get_node_local_name(notUnderstoodHeader));

    return;
}


void 
wsman_generate_encoding_fault( op_t* op, 
                               WsmanFaultDetailType faultDetail ) 
{
    if (op->in_doc == NULL)
        return;
    op->out_doc = wsman_generate_fault(op->cntx, op->in_doc, WSMAN_ENCODING_LIMIT,
            faultDetail, NULL);
    return;
}

int 
wsman_fault_occured(WsmanMessage *msg) 
{
    return (msg->status.fault_code == WSMAN_RC_OK ) ?  0 : 1;
}

