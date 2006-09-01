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


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>


#include "wsman-util.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-dispatcher.h"
#include "wsman-soap-envelope.h"

#include "wsman-xml.h"
#include "wsman-xml-serializer.h" 
#include <libxml/uri.h>
#include "wsman-faults.h"
#include "wsman-debug.h"
#include "wsman-catalog.h"

int  wsman_catalog_create_resource(WsXmlNodeH resource, WsDispatchInterfaceInfo *interface)
{
    WsXmlNodeH r;			
    WsXmlNodeH access;

    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Generating Catalog Resource" );
    r = ws_xml_add_child(resource, NULL, WSMANCAT_RESOURCE, NULL);

    ws_xml_define_ns(r, XML_NS_WS_MAN_CAT, NULL, 1);
    ws_xml_define_ns(r, XML_NS_CIM_SCHEMA, NULL, 0);
    ws_xml_define_ns(r, XML_NS_XML_SCHEMA, NULL, 0);

    ws_xml_add_child(r, 
            NULL, 
            WSMANCAT_RESOURCE_URI, 
            interface->wsmanResourceUri);
    ws_xml_add_child(r, 
            NULL, //XML_NS_WS_MAN_CAT, 
            WSMANCAT_VERSION, 
            interface->version);
    ws_xml_add_child(r, 
            NULL, //XML_NS_WS_MAN_CAT, 
            WSMANCAT_NOTES, 
            interface->notes);
    ws_xml_add_child(r, 
            NULL, //XML_NS_WS_MAN_CAT, 
            WSMANCAT_VENDOR, 
            interface->vendor);

    // Access            
    access = ws_xml_add_child(r, 
            NULL, //XML_NS_WS_MAN_CAT, 
            WSMANCAT_ACCESS, 
            NULL);

    ws_xml_add_child(access, 
            NULL, //XML_NS_WS_MAN_CAT, 
            WSMANCAT_COMPLIANCE, 
            interface->compliance);

    wsman_catalog_add_operations(access, interface );
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Generated Catalog Resource" );

    if (r)
        return 1;    
    return 0;            
}



void wsman_catalog_add_operations(WsXmlNodeH access, WsDispatchInterfaceInfo *interface)
{
    int i;
    WsXmlNodeH operation;
    WsXmlNodeH child;
    WsXmlNodeH child2;
    for(i = 0; interface->endPoints[i].serviceEndPoint != NULL; i++)
    {   
        // Operation            
        operation = ws_xml_add_child(access, 
                NULL, 
                WSMANCAT_OPERATION, 
                NULL);
        child = ws_xml_add_child(operation, 
                NULL,
                WSMANCAT_ACTION, 
                interface->endPoints[i].inAction);

        WsSelector *sel =  interface->endPoints[i].selectors;
        if (sel!=NULL)
        {
            int j;    		
            int set_created = 0;			
            for(j = 0; sel[j].name != NULL; j++)
            {
                if (!set_created) 
                {  		    		    			
                    child = ws_xml_add_child(operation, 
                            NULL, 
                            WSMANCAT_SELECTOR_SET_REF, 
                            NULL);                                          
                    ws_xml_add_node_attr(child, 
                            NULL, 
                            WSM_NAME, 
                            "GetSelector");           				
                    set_created = 1;
                }
            }
        }

    }    

    for(i = 0; interface->endPoints[i].selectors != NULL; i++)
    {
        WsSelector *sel =  interface->endPoints[i].selectors;
        int j;      			
        int set_created = 0;			
        for(j = 0; sel[j].name != NULL; j++)
        {
            if (!set_created) 
            {  		    		    			
                child = ws_xml_add_child(access, 
                        NULL, 
                        WSMANCAT_SELECTOR_SET, 
                        NULL);
                set_created = 1;
            }

            wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Adding Selector: %s", sel[j].name );

            child2 = ws_xml_add_child(child, 
                    NULL,
                    WSMANCAT_SELECTOR, 
                    NULL);

            ws_xml_add_node_attr(child2, 
                    NULL, 
                    WSMANCAT_NAME, 
                    sel[j].name);

            ws_xml_add_qname_attr(child2, 
                    NULL, 
                    WSMANCAT_TYPE, 
                    XML_NS_XML_SCHEMA,
                    sel[j].type);
        }
    }    

}

