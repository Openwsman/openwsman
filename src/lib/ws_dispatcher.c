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


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <glib.h>

#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_api_generic.h"

#include "ws_transport.h"
#include "ws_dispatcher.h"
#include "wsman-debug.h"
#include "wsman-faults.h"




/**
 * @defgroup Dispatcher Dispatcher
 * @brief SOAP Dispatcher
 * 
 * @{
 */


// TBD: ??? Should it be SoapH specific
struct __WkHeaderInfo
{
    char* ns;
    char* name;
};

// IsWkHeader
int is_wk_header(WsXmlNodeH header)
{
    static struct __WkHeaderInfo s_Info[] =
    {
        {XML_NS_ADDRESSING, WSA_TO},
        {XML_NS_ADDRESSING, WSA_MESSAGE_ID},
        {XML_NS_ADDRESSING, WSA_RELATES_TO},
        {XML_NS_ADDRESSING, WSA_ACTION},
        {XML_NS_ADDRESSING, WSA_REPLY_TO},
        {XML_NS_ADDRESSING, WSA_FAULT_TO},
        {XML_NS_WS_MAN, WSM_SYSTEM},
        {XML_NS_WS_MAN, WSM_RESOURCE_URI},
        {XML_NS_WS_MAN, WSM_SELECTOR_SET},
        {XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE},
        {XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT},

        {NULL, NULL}
    };

    int i;
    char* name = ws_xml_get_node_local_name(header);
    char* ns = ws_xml_get_node_name_ns(header);

    for(i = 0; s_Info[i].name != NULL; i++)
    {
        if ( (ns == NULL && s_Info[i].ns == NULL)
                ||
                (ns != NULL && s_Info[i].ns != NULL && !strcmp(ns, s_Info[i].ns)) )
        {
            if ( !strcmp(name, s_Info[i].name) )
                return 1;
        }
    }
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"mustUnderstand: %s:%s", !ns ? "null" : ns, name);  
    return 0;
}


/**
 * Get SOAP Header
 * @param fw SOAP Framework handle
 * @param doc XML document
 * @param nsUri Namespace URI
 * @param name Header element name
 * @return XML node 
 */
// GetSoapHeaderElement
WsXmlNodeH get_soap_header_element(SOAP_FW* fw, 
        WsXmlDocH doc, 
        char* nsUri, 
        char* name)
{
    WsXmlNodeH node = ws_xml_get_soap_header(doc);

    if ( node && name )
    {
        node = ws_xml_find_in_tree(node, nsUri, name, 1);
    }
    return node;
}

/**
 * Get SOAP header value
 * @param fw SOAP Framework handle
 * @param doc XML document
 * @param nsUri Namespace URI
 * @param name Header element name
 * @return Header value
 */
// GetSoapHeaderValue
char* get_soap_header_value(SOAP_FW* fw, WsXmlDocH doc, char* nsUri, char* name)
{
    char* retVal = NULL;
    WsXmlNodeH node = get_soap_header_element(fw, doc, nsUri, name);

    if ( node != NULL )
        retVal = soap_clone_string(ws_xml_get_node_text(node));

    return retVal;
}

/**
 * Check if Message ID is duplicate
 * @param  fw SOAP Framework handle
 * @param doc XML document
 * @return status
 */
// IsDuplicateMsgId
int ws_is_duplicate_message_id (SOAP_FW* fw, WsXmlDocH doc)
{    
    char* msgId = get_soap_header_value(fw, 
            doc, 
            XML_NS_ADDRESSING, 
            WSA_MESSAGE_ID);
            
    int retVal = 0;
	
    if ( msgId )
    {
    		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Checking Message ID: %s", msgId);
        DL_Node* node;
        soap_fw_lock(fw);

        node = DL_GetHead(&fw->processedMsgIdList);

        while( node != NULL )
        {
            // TBD ??? better comparison
            if ( !strcmp(msgId, (char*)node->dataBuf) )
            {              
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
                		"Duplicate Message ID: %s", msgId);                         
                retVal = 1;
                break;
            }
            node = DL_GetNext(node);
        }

        if ( !retVal )
        {
            while( DL_GetCount(&fw->processedMsgIdList) 
                    >= PROCESSED_MSG_ID_MAX_SIZE )
            {
                node = DL_RemoveHead(&fw->processedMsgIdList);
                soap_free(node->dataBuf);
                soap_free(node);
            }

            if ( (node = DL_MakeNode(&fw->processedMsgIdList, NULL)) )
            {
                if ( (node->dataBuf = soap_clone_string(msgId)) == NULL )
                {
                    DL_RemoveNode(node);
                    soap_free(node);
                }
            }
        }

        soap_fw_unlock(fw);
    }
    else
    {       
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "No MessageId found");
    }

    return retVal;
}





/**
 * Check if Envelope is valid
 * @param  fw SOAP Framework handle
 * @param doc XML document
 * @return status
 */
// IsValidEnvelope
WsmanFaultCodeType ws_is_valid_envelope(SOAP_FW* fw, WsXmlDocH doc)
{
    WsmanFaultCodeType fault_code = WSMAN_FAULT_NONE;
    WsXmlNodeH root = ws_xml_get_doc_root(doc);

    if ( !strcmp(SOAP_ENVELOPE, ws_xml_get_node_local_name(root)) )
    {
    	        
        char* soapNsUri = ws_xml_get_node_name_ns(root);		      
        if ( (strcmp(soapNsUri, XML_NS_SOAP_1_2) != 0
                    &&
                    strcmp(soapNsUri, XML_NS_SOAP_1_1) != 0) )
        {
       		// FIXME: Check for message id
        		// build_soap_version_fault(fw);
            fault_code = SOAP_FAULT_VERSION_MISMATCH;
        }
        else
        {
            WsXmlNodeH node = ws_xml_get_soap_body(doc);

            if ( node == NULL ) 
            {
            		wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,"Invalid envelope (no body)"); 
            		fault_code = WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER;               
            }
        }
    }
    return fault_code;
}




// FindResponseEntry
SOAP_OP_ENTRY* find_response_entry(SOAP_FW* fw, char* id)
{
    SOAP_OP_ENTRY* entry = NULL;

    if ( fw )
    {
        DL_Node* node;
        soap_fw_lock(fw);
        node = DL_GetHead(&fw->responseList);

        while( node != NULL )
        {
            entry = (SOAP_OP_ENTRY*)node->dataBuf;
            // TBD: more accurate comparision
            if ( !stricmp(id, entry->dispatch->inboundAction) )
            {
                DL_RemoveNode(node);
                soap_free(node);
                break;
            }
            entry = NULL;

            node = DL_GetNext(node);
        }

        soap_fw_unlock(fw);
    }

 

    return entry;
}

// UnLinkResponseEntry
int unlink_response_entry(SOAP_FW* fw, SOAP_OP_ENTRY* entry)
{
    int retVal = 0;

    if ( fw && entry )
    {
        DL_Node* node;

        soap_fw_lock(fw);

        node = DL_GetHead(&fw->responseList);
        while( node != NULL )
        {
            if ( entry == (SOAP_OP_ENTRY*)node->dataBuf )
            {
                DL_RemoveNode(node);
                soap_free(node);
                retVal = 1;
                break;
            }
            node = DL_GetNext(node);
        }

        soap_fw_unlock(fw);
    }

    return retVal;
}


// ValidateMustUnderstandHeaders
WsXmlNodeH validate_mustunderstand_headers(SOAP_OP_ENTRY* op)
{
    WsXmlNodeH child = NULL;
    WsXmlNodeH header = 
        get_soap_header_element(op->dispatch->fw, op->inDoc, NULL, NULL);
    char* nsUri = ws_xml_get_node_name_ns(header);
    int i;

    for(i = 0; (child = ws_xml_get_child(header, i, NULL, NULL)) != NULL; i++)
    {
        if ( ws_xml_find_attr_bool(child, nsUri, SOAP_MUST_UNDERSTAND) )
        {
            DL_Node* node = DL_GetHead(&op->processedHeaders);
            while(node != NULL)
            {
                if ( node->dataBuf == node )
                    break;
                node = DL_GetNext(node);
            }
            if ( node == NULL )
            {
                if ( !is_wk_header(child) )
                {
                    //retVal = 1;
                    break;
                }
            }
        }
    }

    if ( child != NULL ) 
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
        		"Mustunderstand Fault; %s", ws_xml_get_node_text(child) );
    }
    return child;
}




// ProcessFilterChain
int process_filter_chain(SOAP_OP_ENTRY* op, DL_List* list) 
{
    int retVal = 0;

    SOAP_CALLBACK_ENTRY* filter = (SOAP_CALLBACK_ENTRY*)DL_GetHead(list);
    while( !retVal && filter != NULL )
    {
        retVal = filter->proc((SoapOpH)op, filter->node.dataBuf);
        filter = (SOAP_CALLBACK_ENTRY*)DL_GetNext(&filter->node);
    }
    return retVal;
}


// ProcessFilters
int process_filters(SOAP_OP_ENTRY* op, int inbound)
{
    int retVal = 0;
    DL_List* list;

	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Processing Filters");
    if ( !(op->dispatch->flags & SOAP_SKIP_DEF_FILTERS) )
    {
        list = (!inbound) ? &op->dispatch->fw->outboundFilterList :
            &op->dispatch->fw->inboundFilterList;
        retVal = process_filter_chain(op, list); 
    }

    if ( !retVal )
    {
        list = (!inbound) ? &op->dispatch->outboundFilterList :
            &op->dispatch->inboundFilterList;
        retVal = process_filter_chain(op, list); 
    }

    if ( !retVal && inbound )
    {
        WsXmlNodeH notUnderstoodHeader;
        if ( (notUnderstoodHeader = validate_mustunderstand_headers(op)) != 0 )
        {        
        		wsman_generate_notunderstood_fault(op, notUnderstoodHeader);
        		/*
            WsXmlNodeH child;
            WsXmlNodeH header;
            op->outDoc = ws_xml_create_fault(op->cntx,
                    op->inDoc,
                    "MustUnderstand",
                    NULL,
                    NULL,
                    NULL,
                    "One or more mandatory headers are not understood",
                    NULL,
                    NULL);
            header = ws_xml_get_soap_header(op->outDoc);

            child = ws_xml_add_child(header, XML_NS_SOAP_1_2, "NotUnderstood", NULL);
            ws_xml_add_qname_attr(child, 
                    NULL, 
                    "qname", 
                    ws_xml_get_node_name_ns(notUnderstoodHeader),
                    ws_xml_get_node_local_name(notUnderstoodHeader));
			*/                    

            retVal = 1;
        }
    }
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Filteres Processed: %d", retVal);
    return retVal;
}




/**
 * List all dispatcher interfaces
 * @param interfaces Dispatcher interfaces
 */
void wsman_dispatcher_list( GList *interfaces ) 
{
	GList *node = interfaces;
	while(node) {
		WsDispatchInterfaceInfo* interface = (WsDispatchInterfaceInfo*) node->data;	
		wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,"Listing Dispatcher: interface->wsmanResourceUri: %s", interface->wsmanResourceUri);		 
		node = g_list_next (node);
	}		
}




// ProcessInboundOperation
int process_inbound_operation(SOAP_OP_ENTRY* op)
{
    int retVal = 1;

    if ( process_filters(op, 1) )
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "Inbound filter chain returned error");
        ws_xml_destroy_doc(op->inDoc);
        op->inDoc = NULL;
        if ( (op->dispatch->flags & SOAP_CLIENT_RESPONSE) != 0 )
        {
			retVal = op->dispatch->serviceCallback((SoapOpH)op, op->dispatch->serviceData);
        }
        else
        {
            retVal = soap_submit_op((SoapOpH)op, op->backchannelId, NULL);
            //ws_xml_destroy_doc(op->outDoc);
            //destroy_op_entry(op);
        }
    }
    else
    {
    		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Processing Inbound operation");    	
        if ( op->dispatch->serviceCallback != NULL )
        {
            retVal = op->dispatch->serviceCallback((SoapOpH)op, 
                    op->dispatch->serviceData);			                   
        }
    }
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "retVal=%d", retVal );
    return retVal;
}





// GetRelatesToMsgId
char* get_relates_to_message_id(SOAP_FW* fw, WsXmlDocH doc)
{
    char* msgId = get_soap_header_value(fw, doc, XML_NS_ADDRESSING, WSA_RELATES_TO);
    return msgId;
}



// DispatchInboundCall
void  dispatch_inbound_call(SOAP_CHANNEL *ch)
{   
	
	SOAP_FW* fw = (SOAP_FW*)ch->recvDispatchData;		
	int ret;		
	
    WsXmlDocH inDoc = build_inbound_envelope( fw, ch );
    
	SOAP_OP_ENTRY* op = NULL;	
    if ( inDoc == NULL )
    {
        if ( ch->inputBuffer->associatedTransportData )
        {
                soap_free(ch->inputBuffer->associatedTransportData);
        }
        ch->inputBuffer->associatedTransportData = NULL;  	
    }
    else
    {
    		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Inbound call");
        
        char* relatesTo = get_relates_to_message_id(fw, inDoc);
        if ( relatesTo == NULL ||
                (op = find_response_entry(fw, relatesTo)) == NULL )
        {        	
            SOAP_DISPATCH_ENTRY* dispatch;            

            if ( (dispatch = get_dispatch_entry(fw, ch, inDoc)) != NULL )
            {
                if ( (op = create_op_entry(fw, dispatch, 0)) == NULL )
                {
		    			wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "create_op_entry() failed");
                    destroy_dispatch_entry(dispatch);
                }
            } 
            else 
            {
            		if (!ch->FaultCodeType) 
            		{
           			wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
            				"Fault (No dispatcher entry for this resourceUri)");            			
            			ch->FaultCodeType = WSA_FAULT_DESTINATION_UNREACHABLE;
            			ch->FaultDetailType = WSMAN_FAULT_DETAIL_INVALID_RESOURCEURI;                       	
            		}
            }
        }

        if ( op == NULL )
        {
            if ( ch->inputBuffer->associatedTransportData )
            {
            		soap_free(ch->inputBuffer->associatedTransportData);
                ch->inputBuffer->associatedTransportData = NULL;
            }        	
            ws_xml_destroy_doc(inDoc);
        }
        else
        {   
        		op->backchannelId = ch->uniqueId;   	
            op->inDoc = inDoc;
            
            ret = process_inbound_operation(op);
            if (ret) 
            {
            		wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
            			"Fault (process_inbound_operation error)");
            			
            		ch->FaultCodeType = WSMAN_FAULT_INTERNAL_ERROR;
            		ch->FaultDetailType = WSMAN_FAULT_DETAIL_INVALID_RESOURCEURI;
	    		}
        }
    }      
}


/**
 * Buid Inbound Envelope
 * @param  fw SOAP Framework handle
 * @param buf Message buffer
 * @return XML document with Envelope
 */
WsXmlDocH build_inbound_envelope(SOAP_FW* fw, SOAP_CHANNEL *ch)
{
    WsXmlDocH doc = NULL;   
	char *buf = (char *)soap_alloc(ch->inputBuffer->bufSize + 1, 1);
	strncpy (buf, ch->inputBuffer->buf, ch->inputBuffer->bufSize);	        
    
 	if ( (doc = ws_xml_read_memory((SoapH)fw, buf, strlen(buf), NULL, 0)) != NULL )   
    {        
    		WsmanFaultCodeType fault_code = ws_is_valid_envelope(fw, doc);
    		
        if  ( ws_is_duplicate_message_id(fw, doc) &&  fault_code == WSMAN_FAULT_NONE )
        {            
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
            		"Envelope Discarded: Duplicate MessageID");
            		
            ch->FaultCodeType = WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER;  
            ch->FaultDetailType = WSA_FAULT_DETAIL_DUPLICATE_MESSAGE_ID;          
        }   
        
        if (fault_code != WSMAN_FAULT_NONE)
        {        		        		
            ws_xml_destroy_doc(doc);
            doc = NULL;            
        }        
    }
    else 
    {
    		wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "Parse Error!");
    		ch->FaultCodeType = WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER;
    }
    return doc;
}





// GetDispatchEntry
SOAP_DISPATCH_ENTRY* get_dispatch_entry(SOAP_FW* fw, SOAP_CHANNEL *ch, WsXmlDocH doc)
{
    SOAP_DISPATCH_ENTRY* dispatch = NULL;
    if ( fw->dispatcherProc )
    {    	    
        dispatch = (SOAP_DISPATCH_ENTRY*)
            fw->dispatcherProc(fw->cntx, fw->dispatcherData, doc);
    }

    if ( dispatch == NULL )
    {    	
    		wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Dispatcher Error");    	
        if ( (dispatch = do_get_dispatch_entry(fw, ch, doc)) == NULL )
        {
        		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Dispatcher not defined"); 
        }
    }
    else 
    {
        dispatch->usageCount++;
    }

    return dispatch;
}

// DoGetDispatchEntry
SOAP_DISPATCH_ENTRY* do_get_dispatch_entry(SOAP_FW* fw, SOAP_CHANNEL *ch, WsXmlDocH doc)
{
	
    SOAP_DISPATCH_ENTRY* dispatch = NULL;

    if ( fw )
    {
        int bDestroyAction = 0;
        char* action = get_soap_header_value(fw, 
                doc, 
                XML_NS_ADDRESSING, 
                WSA_ACTION);
                
        if ( action == NULL )
        {                      		
            ch->FaultCodeType = WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER;  
            ch->FaultDetailType = SOAP_FAULT_DETAIL_HEADER_NOT_UNDERSTOOD;           		       	            
        } 
        else 
        {
            soap_fw_lock(fw);
            dispatch = (SOAP_DISPATCH_ENTRY*)DL_GetHead(&fw->dispatchList);
            while( dispatch != NULL )
            {
                char* dispatchAction = dispatch->inboundAction;
				wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE,"Dispatch entry for %s", dispatchAction);
				
                if ( !(dispatch->flags & SOAP_CUSTOM_DISPATCHER) )
                {                	
                    if ( (dispatch->flags & SOAP_ACTION_PREFIX) != 0 )
                    {
                        if ( !strncmp(action, 
                                    dispatchAction, 
                                    strlen(dispatchAction)) )
                        {
                            dispatch->usageCount++;
                            break;
                        }
                    }
                    else
                    {                    	
                        if ( !strcmp(action, dispatchAction) )
                        {
                            dispatch->usageCount++;
                            break;
                        }
                    }
                }					
                dispatch = 
                    (SOAP_DISPATCH_ENTRY*)DL_GetNext(&dispatch->node);
            }

            soap_fw_unlock(fw);

            if ( dispatch == NULL )
            {
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
                		"No dispatch entry for %s", action);
            }

            if ( bDestroyAction ) 
            {
                soap_free(action);
            }
        }
    }

    return dispatch;
}


// WsManDispatcher
SoapDispatchH wsman_dispatcher(WsContextH cntx, void* data, WsXmlDocH doc)
{	
    SoapDispatchH disp = NULL;
    if ( doc == NULL )
    {
        // Do clean up on exit
        soap_free(data);
    }
    else
    {
        WsManDispatcherInfo* dispInfo = (WsManDispatcherInfo*)data;
       	wsman_dispatcher_list(dispInfo->interfaces);
        WsDispatchEndPointInfo* ep = NULL;
        char* uri = wsman_get_resource_uri(cntx, doc);
        char* action = ws_addressing_get_action(cntx, doc);
        int i; 
        int resUriMatch = 0;

		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"Dispatcher: %s, %s", uri, action);
        if ( uri && action )
        {
            WsDispatchInterfaceInfo* r = NULL;  
                                              
			wsman_debug (WSMAN_DEBUG_LEVEL_MESSAGE, 
				"Registered interfaces: %d", g_list_length(dispInfo->interfaces) );
				
			GList *node = dispInfo->interfaces;
            while(node!=NULL)
            {            
            		WsDispatchInterfaceInfo* interface = (WsDispatchInterfaceInfo*)node->data;
            	
            		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"Dispatcher: uri: %s, interface->wsmanResourceUri: %s", uri, interface->wsmanResourceUri);
            	
                if ( !strcmp(uri, interface->wsmanResourceUri) )
                {     
                		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,
                			"Dispatcher interface match: %s", interface->wsmanResourceUri);        	                                       
                    r = interface;
                    resUriMatch = 1;
                    break;                    
                }
                node = g_list_next (node);                            
            }
            
            if ( r != NULL )
            {            	
                char* ptr = action;

                if ( r->actionUriBase )
                {
                    int len = strlen(r->actionUriBase);
                    if ( !strncmp(action, r->actionUriBase, len) && action[len] == '/' )
                        ptr = &action[len + 1];
                }

                for(i = 0; r->endPoints[i].serviceEndPoint != NULL; i++)
                {
                    if ( !strcmp(ptr, r->endPoints[i].inAction) )
                    {
                        ep = &r->endPoints[i];
                        break;
                    }
                }
            }
        }

        if ( uri )
        {
            ws_remove_context_val(cntx, WSM_RESOURCE_URI);
        }

        if ( ep != NULL )
        {
            for(i = 0; i < dispInfo->mapCount; i++)
            {
                if ( dispInfo->map[i].ep == ep )
                {
                    disp = dispInfo->map[i].disp;
                    break;
                }
            }
        } 
        else 
        {
        		if (resUriMatch == 1 ) 
        		{
        			wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,"No endpoint found for this action");
             	//ch->FaultCodeType = WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER;  
            		//ch->FaultDetailType = WSA_FAULT_DETAIL_DUPLICATE_MESSAGE_ID;  	
        			// TBD: detailed fault as in spec.
        			/*
        			WsSoapMessageH* ws_message = (WsSoapMessageH*)message;
            		WsXmlDocH fault = ws_xml_create_fault(cntx,
                        doc,
                        "Sender",
                        XML_NS_WS_MAN,
                        "ActionMismatch",
                        NULL,
                        "Action not supported for this resource",
                        NULL,
                        NULL);
                        
            		build_soap_message_response(fault, ws_message );                    
            		*/
        		}
        }
    }	
    return disp;
}


/** @} */


