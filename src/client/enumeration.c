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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

#include "ws_utilities.h"


#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"

#include "wsman-debug.h"
#include "wsman.h"


WsXmlDocH wsman_make_enum_message(WsContextH soap, 
        char* op, 
        char* enumContext,
        char* resourceUri,
        char* url)
{
    char* action = wsman_make_action(XML_NS_ENUMERATION, op);
    WsXmlDocH doc = wsman_build_envelope(soap, 
            action, 
            WSA_TO_ANONYMOUS, 
            NULL, 
            resourceUri,
            url,
            60000,
            50000); 
    if ( doc != NULL )
    {
        WsXmlNodeH node = ws_xml_add_child(ws_xml_get_soap_body(doc), 
                XML_NS_ENUMERATION, 
                op, 
                NULL);
        if ( enumContext ) 
        {
            ws_xml_add_child(node, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT, enumContext);
        }
    }

    free(action);
    return doc;
}



WsXmlNodeH wsman_enum_send_get_response(WsClientContextH *ctx, 
        char* op, 
        char* enumContext, 
        char* resourceUri,
        char* url)
{
    WsXmlNodeH node = NULL;
    WsXmlDocH rqstDoc = wsman_make_enum_message(ctx->wscntx, 
            op, 
            enumContext,
            resourceUri,
            url);

    if ( rqstDoc )
    {
        WsXmlDocH respDoc;
     
        //ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(rqstDoc), 1);
        respDoc = _ws_send_get_response(ctx, rqstDoc, url);

        if ( respDoc )
        {           
            ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(respDoc), 1);
            node = ws_xml_get_child(ws_xml_get_soap_body(respDoc), 0, NULL, NULL);
            if ( node == NULL && respDoc != NULL )
                ws_xml_destroy_doc(respDoc);
        }
        else 
        {
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "ws_send_get_response failed");
        }
        ws_xml_destroy_doc(rqstDoc);
    }
    else 
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "wsman_build_envelope failed");
    }
    return node;
}



int wsman_enumeration(char* url, 
        WsClientContextH *ctx, 
        char *resourceUri,
        int count)
{
	int retVal = 0;
    char* enumContext = NULL;
   
    WsXmlNodeH enumStartNode = wsman_enum_send_get_response(ctx, 
            WSENUM_ENUMERATE, 
            NULL, 
            resourceUri,
            url);

    if ( enumStartNode )
    {    		
        WsXmlNodeH node;
        WsXmlNodeH cntxNode = ws_xml_get_child(enumStartNode, 
                0, 
                XML_NS_ENUMERATION, 
                WSENUM_ENUMERATION_CONTEXT);
        enumContext = soap_clone_string(ws_xml_get_node_text(cntxNode));

        if ( enumContext || (enumContext && enumContext[0] == 0) )
        {
            while( count && (node = wsman_enum_send_get_response(ctx, 
                            WSENUM_PULL, 
                            enumContext, 
                            resourceUri,
                            url)) )
            {
                if ( strcmp(ws_xml_get_node_local_name(node), WSENUM_PULL_RESP) != 0 )
                {
                    ws_xml_destroy_doc(ws_xml_get_node_doc(node));
                    break;
                }

                if ( ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE) )
                {
                    ws_xml_destroy_doc(ws_xml_get_node_doc(node));
                    break;
                }
                if ( (cntxNode = ws_xml_get_child(node, 
                                0, 
                                XML_NS_ENUMERATION, 
                                WSENUM_ENUMERATION_CONTEXT)) )
                {
                    soap_free(enumContext);
                    enumContext = soap_clone_string(ws_xml_get_node_text(cntxNode));
                }

                ws_xml_destroy_doc(ws_xml_get_node_doc(node));
                count--;
                if ( enumContext == NULL || enumContext[0] == 0 )
                {
                    wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "No enumeration context");
                    break;
                }
            }
            if ( !count ) 
            {
                node = wsman_enum_send_get_response(ctx, 
                        WSENUM_RELEASE, 
                        enumContext, 
                        resourceUri,
                        url);
                ws_xml_destroy_doc(ws_xml_get_node_doc(node));
            }
        }
        else
        {
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "No enumeration context ???");
        }

        ws_xml_destroy_doc(ws_xml_get_node_doc(enumStartNode));
    } 
    else 
    {
    		
    		retVal = 1;
    }
    return retVal;
}

