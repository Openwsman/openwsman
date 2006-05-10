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
#include <glib.h>

#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "ws_dispatcher.h"

#include "xml_api_generic.h"
#include "xml_serializer.h" 
#include "wsman-faults.h"
#include "wsman-debug.h"


static 
char *get_fault_details(WsmanFaultDetailType faultDetail)
{
    char *descr;
    switch (faultDetail) 
    {
    case WSMAN_FAULT_DETAIL_INVALID_RESOURCEURI:
        descr = 
            "The WS-Management service cannot process the request." \
            " The specified resource URI is missing or in an incorrect format.";
        break;
    case OWSMAN_FAULT_DETAIL_ENDPOINT_ERROR:
        descr = "Endpoint Failure";
        break;
    default:
        descr = NULL;
        break;
    }
    return descr;
}




/**
 * Build SOAP Fault
 * @param  fw SOAP Framework handle
 * @param soapNsUri SOAP Namespace URI
 * @param faultNsUri Fault Namespace URI
 * @param code Fault code
 * @param subCode Fault Subcode
 * @param reason Fault Reson
 * @param detail Fault Details
 * @return Fault XML document
 */
// BuildSoapFault
WsXmlDocH build_soap_fault(SOAP_FW* fw,
        char* soapNsUri,
        char* faultNsUri,
        char* code,
        char* subCode,
        char* reason,
        char* detail)
{
    WsXmlDocH doc;

    if ( faultNsUri == NULL )
        faultNsUri = soapNsUri;

    if ( (doc = ws_xml_create_doc((SoapH)fw, soapNsUri, SOAP_ENVELOPE)) != NULL ) 
    {
        WsXmlNodeH node;
        WsXmlNodeH fault;
        WsXmlNodeH root = ws_xml_get_doc_root(doc);
        //WsXmlNodeH header = WsXmlAddChild(root, soapNsUri, SOAP_HEADER, NULL);
        WsXmlNodeH body = ws_xml_add_child(root, soapNsUri, SOAP_BODY, NULL);

        ws_xml_define_ns(root, soapNsUri, NULL, 0);
        ws_xml_define_ns(root, XML_NS_ADDRESSING, NULL, 0);
        ws_xml_define_ns(root, XML_NS_XML_NAMESPACES, NULL, 0);
        if ( strcmp(soapNsUri, faultNsUri) != 0 ) 
            ws_xml_define_ns(root, faultNsUri, NULL, 0);

        if ( body && (fault = ws_xml_add_child(body, soapNsUri, SOAP_FAULT, NULL)) )
        {
            if ( code != NULL 
                    &&
                    (node = ws_xml_add_child(fault, soapNsUri, SOAP_CODE, NULL)) != NULL )
            {
                ws_xml_add_qname_child(node, soapNsUri, SOAP_VALUE, soapNsUri, code); 

                if ( subCode != NULL
                        && 
                        (node = ws_xml_add_child(node, soapNsUri, SOAP_SUBCODE, NULL)) != NULL )
                {
                    ws_xml_add_qname_child(node, soapNsUri, SOAP_VALUE, faultNsUri, subCode); 
                }
            }

            if ( reason && (node = ws_xml_add_child(fault, soapNsUri, SOAP_REASON, NULL)) )
            {
                node = ws_xml_add_child(node, soapNsUri, SOAP_TEXT, reason);
                ws_xml_add_node_attr(node, XML_NS_XML_NAMESPACES, SOAP_LANG, "en");
            }

            if ( detail )
                ws_xml_add_child(fault, soapNsUri, SOAP_DETAIL, detail);
        }
    }

    return doc;
}




/**
 * Buid SOAP Version Mismtach Fault
 * @param  fw SOAP Framework handle
 * @todo Send fault back
 */     
void build_soap_version_fault(SOAP_FW* fw)
{
    WsXmlDocH fault = build_soap_fault(fw, 
            NULL, 
            XML_NS_SOAP_1_2, 
            "VersionMismatch", 
            NULL, 
            "Version Mismatch", 
            NULL);

    if ( fault != NULL )
    {
        WsXmlNodeH upgrade;
        WsXmlNodeH h = ws_xml_get_soap_header(fault);

        ws_xml_define_ns(ws_xml_get_doc_root(fault), XML_NS_SOAP_1_1, NULL, 0);

        if ( (upgrade = ws_xml_add_child(h, XML_NS_SOAP_1_2, SOAP_UPGRADE, NULL)) )
        {
            WsXmlNodeH node;

            if ( (node = ws_xml_add_child(upgrade, 
                            XML_NS_SOAP_1_2, 
                            SOAP_SUPPORTED_ENVELOPE, 
                            NULL)) )
            {
                ws_xml_add_qname_attr(node, NULL, "qname", XML_NS_SOAP_1_2, SOAP_ENVELOPE);
            }
            if ( (node = ws_xml_add_child(upgrade, 
                            XML_NS_SOAP_1_2, 
                            SOAP_SUPPORTED_ENVELOPE, 
                            NULL)) )
            {
                ws_xml_add_qname_attr(node, NULL, "qname", XML_NS_SOAP_1_1, SOAP_ENVELOPE);
            }
        }


        // FIXME: Send fault
        ws_xml_destroy_doc(fault);
    }
}   





WsXmlDocH wsman_generate_fault(
        WsContextH cntx, 
        WsXmlDocH inDoc, 
        WsmanFaultCodeType faultCode, 
        WsmanFaultDetailType faultDetail)
{
    char *code = FAULT_SENDER_CODE;
    char *subCodeNs;
    char *subCode;
    char *reason;
    char *detail = NULL;

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Fault Code: %d", faultCode);
    switch (faultCode) 
    {
    case WSMAN_FAULT_INVALID_SELECTORS:
        subCodeNs 	= XML_NS_WS_MAN;
        subCode		= "InvalidSelectors";
        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "The selectors for the resource were not valid";

        break;

    case SOAP_FAULT_MUSTUNDERSTAND:
        subCodeNs 	= XML_NS_SOAP_1_2;
        subCode		= "MustUnderstand"; 		 			
        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Header not understood";
        break; 				 		 		 		
    case WSMAN_FAULT_INTERNAL_ERROR:
        subCodeNs 	= XML_NS_WS_MAN;
        subCode		= "InternalError";
        code			= FAULT_RECEIVER_CODE;

        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Internal Error."; 		 			
        break;
    case WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER:
        subCodeNs 	= XML_NS_ADDRESSING;
        subCode 	  	= "InvalidMessageInformationHeader";

        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "Invalid Header"; 			 		
        break;
    case WSEN_FAULT_INVALID_ENUMERATION_CONTEXT:
        subCodeNs 	= XML_NS_ENUMERATION;
        subCode 	  	= "InvalidEnumerationContext";
        code			= FAULT_RECEIVER_CODE;
        if (( reason = get_fault_details(faultDetail)) == NULL)
            reason 		= "An invalid enumeration context was supplied with the message"; 			 		
        break; 		
    case WSA_FAULT_DESTINATION_UNREACHABLE:
        detail 		= "faultDetail/InvalidResourceURI";
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
    WsXmlDocH fault = ws_xml_create_fault(cntx,
            inDoc,
            code,
            subCodeNs,
            subCode,
            NULL,
            reason,
            add_details_proc,
            detail);	                    
    return fault;                        
}



void add_details_proc(WsXmlNodeH fault,  void* data) 
{
    if (data == NULL)
        return;
    char* soapNs = ws_xml_get_node_name_ns(fault);
    WsXmlNodeH node = ws_xml_add_child(fault, soapNs, SOAP_DETAIL, NULL);	
    node = ws_xml_add_child(node, XML_NS_WS_MAN, SOAP_FAULT_DETAIL, NULL);
    ws_xml_set_node_qname_val(node, XML_NS_WS_MAN, (char *)data);
    return;
}



void wsman_generate_fault_buffer
(
 WsContextH cntx, 
 WsXmlDocH inDoc, 
 WsmanFaultCodeType faultCode, 
 WsmanFaultDetailType faultDetail,
 char **buf, 
 int* len)
{	
    WsXmlDocH doc = wsman_generate_fault(cntx, inDoc,faultCode, faultDetail);   
    ws_xml_dump_memory_enc(doc, buf, len, NULL);	
    return;
}



void wsman_generate_notunderstood_fault(
        SOAP_OP_ENTRY* op, 
        WsXmlNodeH notUnderstoodHeader
        ) 
{
    WsXmlNodeH child;
    WsXmlNodeH header;

    if (op->inDoc == NULL)
        return;
    op->outDoc = wsman_generate_fault(op->cntx,
            op->inDoc,
            SOAP_FAULT_MUSTUNDERSTAND,
            SOAP_FAULT_DETAIL_HEADER_NOT_UNDERSTOOD
            );


    header = ws_xml_get_soap_header(op->outDoc);

    child = ws_xml_add_child(header, XML_NS_SOAP_1_2, "NotUnderstood", NULL);
    ws_xml_add_qname_attr(child, 
            NULL, 
            "qname", 
            ws_xml_get_node_name_ns(notUnderstoodHeader),
            ws_xml_get_node_local_name(notUnderstoodHeader));

    return;
}


