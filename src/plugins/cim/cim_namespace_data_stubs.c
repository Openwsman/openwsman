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

#include "config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include <gmodule.h>

#include "ws_utilities.h"



#include "ws_errors.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"
#include "ws_dispatcher.h"

#include "wsman-debug.h"
#include "sfcc-interface.h"
#include "sfcc-interface_utils.h"
#include "cim_namespace_data.h"

int  CimResource_Custom_EP(SoapOpH op, void* appData ) {
    WsXmlDocH doc = NULL;
    WsmanStatus *status = (WsmanStatus *)soap_alloc(sizeof(WsmanStatus *), 0 );

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Custom Method Endpoint Called");

    SoapH soap = soap_get_op_soap(op);
    WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
    GList *keys = wsman_get_selector_list(cntx, NULL);
    char *resourceUri = wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL));
    char *action = ws_addressing_get_action(cntx, NULL);
    CimClientInfo cimclient;

    if (keys)
    {
        cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL , status);
        if (!cimclient.cc) {
            return 1;		
        }

        char *className = resourceUri + strlen(XML_NS_CIM_V2_9) + 1;
        char *methodName = action + strlen(resourceUri) + 1;
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Class Name: %s, Method: %s" , className, methodName);
        
        if ( (doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL)) ) {    		
            WsXmlNodeH method_node = ws_xml_add_empty_child_format(ws_xml_get_soap_body(doc), resourceUri , "%s_OUTPUT", methodName);       
            cim_invoke_method(cimclient.cc, className, keys, methodName, method_node,  status);
        }
        
        if (status->rc != 0) {
            ws_xml_destroy_doc(doc);
            doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), status->rc, -1);
            if (cimclient.cc) CMRelease(cimclient.cc);
        }
    } else {
        doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), WSMAN_FAULT_INVALID_SELECTORS, -1);
    } 

    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
        soap_submit_op(op);
    } else {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Invalid doc" );
    }
    ws_destroy_context(cntx);
    if (status)
        soap_free(status);
    return 0;
}


int  CimResource_Get_EP(SoapOpH op, void* appData )
{
    WsXmlDocH doc = NULL;
    WsmanStatus *status = (WsmanStatus *)soap_alloc(sizeof(WsmanStatus *), 0 );
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Get Endpoint Called");

    SoapH soap = soap_get_op_soap(op);
    WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
    GList *keys = wsman_get_selector_list(cntx, NULL);
    char *resourceUri = wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL));

    if (keys)
    {
        wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Number of keys defined: %d", g_list_length(keys));
        CimClientInfo cimclient;
        cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL , status);
        if (!cimclient.cc)
            return 1;		
        if ( (doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL)) ) {    		
            WsXmlNodeH body = ws_xml_get_soap_body(doc);
            cim_get_instance(cimclient.cc, resourceUri , keys, body, status);
        }

        if (status->rc != 0) {
            ws_xml_destroy_doc(doc);
            doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), status->rc, -1);
        }
        
        if (cimclient.cc) CMRelease(cimclient.cc);
    } else {
        doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), WSMAN_FAULT_INVALID_SELECTORS, -1);
    } 

    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
        soap_submit_op(op);
    } else {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Invalid doc" );
    }

    soap_free(status);
    return 0;

}


int CimResource_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo, WsmanStatus *status)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Enumerate Endpoint Called"); 
    char *resourceUri = wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL));
    char *className = resourceUri + sizeof(XML_NS_CIM_V2_9);

    CimClientInfo cimclient;
    cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL , NULL);
    if (!cimclient.cc) {
        return 1;
    }
    cim_enum_instances (cimclient.cc, className , enumInfo,  status);
    if (status && status->rc != 0) {
        return 1;
    }
    if (cimclient.cc) CMRelease(cimclient.cc);
    ws_destroy_context(cntx);
    return 0;
}

int CimResource_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo, WsmanStatus *status)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Release Endpoint Called");      
    cim_release_enum_context(enumInfo);
    return 0;
}

int CimResource_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo, WsmanStatus *status)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Pull Endpoint Called");      
    WsXmlDocH doc = NULL;
    WsXmlNodeH itemsNode = NULL;

    char *resourceUri = wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL));
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Resource Uri: %s", resourceUri); 

    int max = wsen_get_max_elements(cntx, NULL);
    doc = ws_create_response_envelope(cntx, ws_get_context_xml_doc_val(cntx, WSFW_INDOC), NULL);
    WsXmlNodeH pullnode = ws_xml_add_child(ws_xml_get_soap_body(doc), XML_NS_ENUMERATION, 
            WSENUM_PULL_RESP, NULL);       

    if ( pullnode != NULL ) {
        itemsNode = ws_xml_add_child(pullnode, XML_NS_ENUMERATION, WSENUM_ITEMS, NULL);     	
    }   
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Total items: %lu", enumInfo->totalItems );
    if (max > 0 ) {
        while(max > 0 && enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems) {
            cim_getElementAt(enumInfo, itemsNode, resourceUri);
            enumInfo->index++;
            max--;
        }
        enumInfo->index--;
    } else {
        if ( enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems ) {
            cim_getElementAt(enumInfo, itemsNode, resourceUri);
        }
    }

    // Filter
    //

    check_xpath(itemsNode, NULL);

    if (doc != NULL )
        enumInfo->pullResultPtr = doc;
    else
        enumInfo->pullResultPtr = NULL;

    ws_destroy_context(cntx);
    return 0;
}





