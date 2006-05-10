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

#include "wsman_catalog.h"
#include "wsman-debug.h"




int WsManCatalog_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Pull Endpoint Called");      
    WsXmlDocH doc = NULL;
    WsXmlNodeH itemsNode = NULL;
    int retVal;

    SOAP_FW* fw = (SOAP_FW*)ws_context_get_runtime(cntx);
    WsManDispatcherInfo* dispInfo = (WsManDispatcherInfo*)fw->dispatcherData;
    doc = ws_create_response_envelope(cntx, ws_get_context_xml_doc_val(cntx, WSFW_INDOC), NULL);

    WsXmlNodeH pullnode = ws_xml_add_child(ws_xml_get_soap_body(doc), 
            XML_NS_ENUMERATION, 
            WSENUM_PULL_RESP,
            NULL);       

    if ( pullnode != NULL )
    {
        itemsNode = ws_xml_add_child(pullnode, 
                XML_NS_ENUMERATION, 
                WSENUM_ITEMS,
                NULL);     	
    }   


    GList *node = dispInfo->interfaces;
    while (node) 
    {
        WsDispatchInterfaceInfo* interface = (WsDispatchInterfaceInfo*) node->data;	
        wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,"Interface: %s", interface->wsmanResourceUri);				 		
        retVal = wsmancat_create_resource(itemsNode, interface);									
        node = g_list_next (node);
    }

    if (doc != NULL )
    {
        enumInfo->pullResultPtr = doc;
    } 
    return 0;
}

int WsManCatalog_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Enumerate Endpoint Called");      
    return 0;
}

int WsManCatalog_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Release Endpoint Called");      
    return 0;
}

int WsManCatalog_Get_EP(SoapOpH op, void* appData)
{
    // Get interfaces
    int retVal = 0;
    WsXmlDocH doc = NULL;
    SoapH soap = soap_get_op_soap(op);
    WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));

    SOAP_FW* fw = (SOAP_FW*)ws_context_get_runtime(cntx);
    WsManDispatcherInfo* dispInfo = (WsManDispatcherInfo*)fw->dispatcherData;

    char* key = wsman_get_selector(cntx, NULL, WSMANCAT_RESOURCE_URI, 0);
    if (key)
    {
        GList *node = dispInfo->interfaces;
        while (node) 
        {
            WsDispatchInterfaceInfo* interface = (WsDispatchInterfaceInfo*) node->data;	
            wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,
                    "Interface: %s", interface->wsmanResourceUri);

            if (!strcmp(interface->wsmanResourceUri, key )) 
            {
                if ( (doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL)) )
                {    		
                    WsXmlNodeH r = ws_xml_add_child(ws_xml_get_soap_body(doc), NULL, WSMANCAT_RESOURCE, NULL);
                    retVal = wsmancat_create_resource(r, interface);
                }
                break;			
            }
            node = g_list_next (node);
        }	
    }

    if (!key || !retVal)
    {
        doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL);
        doc = wsman_generate_fault(
                cntx, 
                doc, 
                WSMAN_FAULT_INVALID_SELECTORS,
                -1);
    }

    if ( doc )
    {
        soap_set_op_doc(op, doc, 0);
        soap_submit_op(op, soap_get_op_channel_id(op), NULL);
        //ws_xml_destroy_doc(doc);
    } 
    else 
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Invalid doc" );
    }

    return 0;
}



