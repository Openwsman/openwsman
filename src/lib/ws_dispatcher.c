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


#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <glib.h>

#include "ws_utilities.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_api_generic.h"

#include "ws_dispatcher.h"
#include "xml_serializer.h"
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
WsXmlNodeH get_soap_header_element(SOAP_FW* fw, 
        WsXmlDocH doc, char* nsUri, char* name)
{
    WsXmlNodeH node = ws_xml_get_soap_header(doc);
    if ( node && name ) {
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
int ws_is_duplicate_message_id (SOAP_FW* fw, WsXmlDocH doc)
{    
    char* msgId = get_soap_header_value(fw, doc, XML_NS_ADDRESSING, WSA_MESSAGE_ID);

    int retVal = 0;
    if ( msgId ) {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Checking Message ID: %s", msgId);
        DL_Node* node;
        soap_fw_lock(fw);
        node = DL_GetHead(&fw->processedMsgIdList);

        while( node != NULL )
        {
            // TBD ??? better comparison
            if ( !strcmp(msgId, (char*)node->dataBuf) ) {              
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Duplicate Message ID: %s", msgId);                         
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
    } else {       
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "No MessageId found");
    }
    free(msgId);

    return retVal;
}





/**
 * Check if Envelope is valid
 * @param  fw SOAP Framework handle
 * @param doc XML document
 * @return status
 */
WsmanFaultCodeType ws_is_valid_envelope(SOAP_FW* fw, WsXmlDocH doc)
{
    WsmanFaultCodeType fault_code = WSMAN_RC_OK;
    WsXmlNodeH root = ws_xml_get_doc_root(doc);

    if ( !strcmp(SOAP_ENVELOPE, ws_xml_get_node_local_name(root)) )
    {
        char* soapNsUri = ws_xml_get_node_name_ns(root);		      
        if ( (strcmp(soapNsUri, XML_NS_SOAP_1_2) != 0
                    && strcmp(soapNsUri, XML_NS_SOAP_1_1) != 0) )
        {
            fault_code = SOAP_FAULT_VERSION_MISMATCH;
        } else {
            WsXmlNodeH node = ws_xml_get_soap_body(doc);
            if ( node == NULL ) {
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,"Invalid envelope (no body)"); 
                fault_code = WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER;               
            }
        }
    }
    return fault_code;
}


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

int unlink_response_entry(SOAP_FW* fw, SOAP_OP_ENTRY* entry)
{
    int retVal = 0;

    if ( fw && entry )
    {
        DL_Node* node;

        gboolean try = soap_fw_trylock(fw);

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

        if (!try)
            soap_fw_unlock(fw);
    }

    return retVal;
}

int validate_control_headers(SOAP_OP_ENTRY* op) {
    unsigned long size = 0;
    WsXmlNodeH header = 
        get_soap_header_element(op->dispatch->fw, op->inDoc, NULL, NULL);
    if ( ws_xml_get_child(header, 0, XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE) != NULL )
    {
        size = ws_deserialize_uint32(NULL, header, 0, XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE);
        if ( size < WSMAN_MINIMAL_ENVELOPE_SIZE_REQUEST) {
            return 1;
        }
    }
    return 0;
}


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

/**
 * Process Filters
 * @param op SOAP operation
 * @param inbound Direction of message, 0 for outbound  and 1 for inbound.
 * @return 0 on sucesses, 1 on error.
 **/
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
            retVal = 1;
        }
        if (validate_control_headers(op)) {
            
            wsman_generate_encoding_fault(op, WSMAN_FAULT_DETAIL_MAX_ENVELOPE_SIZE);
            retVal = 1;
        }
    }
    if (retVal) {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Filteres Processed");
    }
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



void wsman_create_identify_response(SOAP_FW *fw, WsmanMessage *msg) {

    WsXmlDocH doc = ws_xml_create_envelope(ws_context_get_runtime(fw->cntx), NULL);
    WsXmlNodeH identify = ws_xml_add_child(ws_xml_get_soap_body(doc), XML_NS_WSMAN_ID, WSMID_IDENTIFY_RESPONSE , NULL);
    ws_xml_add_child(identify, XML_NS_WSMAN_ID, WSMID_PROTOCOL_VERSION , XML_NS_WS_MAN);
    ws_xml_add_child(identify, XML_NS_WSMAN_ID, WSMID_PRODUCT_VENDOR , PACKAGE_NAME);
    ws_xml_add_child(identify, XML_NS_WSMAN_ID, WSMID_PRODUCT_VERSION , PACKAGE_VERSION);
    char *buf = NULL;
    int len;
    ws_xml_dump_memory_enc(doc, &buf, &len, "UTF-8");
    msg->response.length = len;
    msg->response.body = strndup(buf, len);

    soap_free(buf);
    ws_xml_destroy_doc(doc);
    return;
}

int process_inbound_operation(SOAP_OP_ENTRY* op, WsmanMessage *msg)
{
    int retVal = 1;
    char* buf = NULL;
    int len;
    if ( process_filters(op, 1) ) {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "Inbound filter chain returned error");
        if (op->outDoc) {
            ws_xml_dump_memory_enc(op->outDoc, &buf, &len, "UTF-8");
            msg->response.length = len;
            msg->response.body = strndup(buf, len);

            ws_xml_destroy_doc(op->outDoc);
            soap_free(buf);
            destroy_op_entry(op);
        } else {
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "doc is null");
        }
    } else {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Processing Inbound operation");    	
        if ( op->dispatch->serviceCallback != NULL )
            retVal = op->dispatch->serviceCallback((SoapOpH)op, op->dispatch->serviceData);
        else
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "op is null");    	

        if ( (retVal = process_filters(op, 0)) == 0 ) {        	
            if (op->outDoc) {
                ws_xml_dump_memory_enc(op->outDoc, &buf, &len, "UTF-8");
                msg->response.length = len;
                msg->response.body = strndup(buf, len);

                ws_xml_destroy_doc(op->outDoc);
                soap_free(buf);
                destroy_op_entry(op);
            } else {
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "doc is null");
            }
        }
    }
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "retVal=%d", retVal );
    return retVal;
}


char* get_relates_to_message_id(SOAP_FW* fw, WsXmlDocH doc)
{
    char* msgId = get_soap_header_value(fw, doc, XML_NS_ADDRESSING, WSA_RELATES_TO);
    return msgId;
}



void dispatch_inbound_call(SOAP_FW *fw, WsmanMessage *msg) 
{   
    int ret;		
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Inbound call");
    WsXmlDocH inDoc = build_inbound_envelope( fw, msg);

    if (wsman_is_identify_request(inDoc)) {
        msg->in_doc = inDoc;
        wsman_create_identify_response(fw, msg);
        return;
    }

    SOAP_OP_ENTRY* op = NULL;	
    if ( inDoc != NULL && msg->status.rc == WSMAN_RC_OK) 
    {
        char* relatesTo = get_relates_to_message_id(fw, inDoc);

        if ( relatesTo == NULL || (op = find_response_entry(fw, relatesTo)) == NULL ) {        	
            SOAP_DISPATCH_ENTRY* dispatch;            
            if ( (dispatch = get_dispatch_entry(fw, inDoc)) != NULL ) {
                if ( (op = create_op_entry(fw, dispatch, 0)) == NULL ) {
                    wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "create_op_entry() failed");
                    destroy_dispatch_entry(dispatch);
                }
            } else {
		wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "msg->status.rc=%d", msg->status.rc );
                if (!msg->status.rc) {
                    wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Fault (No dispatcher entry for this resourceUri)");            			
                    wsmand_set_fault(msg, WSA_FAULT_DESTINATION_UNREACHABLE, WSMAN_FAULT_DETAIL_INVALID_RESOURCEURI, NULL);
                }
            }
        }

        if ( op != NULL ) {
            op->inDoc = inDoc;
            ret = process_inbound_operation(op, msg);
            if (ret) {
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Fault (process_inbound_operation error)");
            }
        }
    }

    if (inDoc != NULL) {
        msg->in_doc = inDoc;
    }
}


int wsman_is_identify_request(WsXmlDocH doc) {

    WsXmlNodeH node = ws_xml_get_soap_body(doc);
    node = ws_xml_get_child(node, 0, XML_NS_WSMAN_ID, WSMID_IDENTIFY);
    if (node)
        return 1;
    else
        return 0;
}

/**
 * Buid Inbound Envelope
 * @param  fw SOAP Framework handle
 * @param buf Message buffer
 * @return XML document with Envelope
 */
WsXmlDocH build_inbound_envelope(SOAP_FW* fw, WsmanMessage *msg)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Building inbound envelope");
    WsXmlDocH doc = NULL;   
    if ( (doc = ws_xml_read_memory((SoapH)fw,  msg->request.body, msg->request.length, NULL, 0)) != NULL )   
    {        
        WsmanFaultCodeType fault_code = ws_is_valid_envelope(fw, doc);
        if (!wsman_is_identify_request(doc)) {
            if  ( ws_is_duplicate_message_id(fw, doc) &&  fault_code == WSMAN_RC_OK ) {
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Envelope Discarded: Duplicate MessageID");
                fault_code = WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER;
                wsmand_set_fault(msg, WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER, WSA_FAULT_DETAIL_DUPLICATE_MESSAGE_ID, NULL);
            }
        }
    } else {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "Parse Error!");
        wsmand_set_fault(msg, WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER, 0, NULL);
        ws_xml_destroy_doc(doc);
    }
    return doc;
}


/**
 * Buid Inbound Envelope
 * @param  fw SOAP Framework handle
 * @param buf Message buffer
 * @return XML document with Envelope
 */
WsXmlDocH wsman_build_inbound_envelope(SOAP_FW* fw, char *inputBuffer, int inputBufferSize)
{
    WsXmlDocH doc = NULL;   
    char *buf = (char *)soap_alloc( inputBufferSize + 1, 1);
    strncpy (buf, inputBuffer, inputBufferSize);	        

    if ( (doc = ws_xml_read_memory((SoapH)fw, buf, strlen(buf), NULL, 0)) != NULL )   
    {        
        WsmanFaultCodeType fault_code = ws_is_valid_envelope(fw, doc);

        if  ( ws_is_duplicate_message_id(fw, doc) &&  fault_code == WSMAN_RC_OK )
        {            
            wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, 
                    "Envelope Discarded: Duplicate MessageID");    
        }   

        if (fault_code != WSMAN_RC_OK)
        {        		        		
            ws_xml_destroy_doc(doc);
            doc = NULL;            
        }        
    }
    else 
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR , "Parse Error!");    		
    }
    free(buf);
    return doc;
}


SOAP_DISPATCH_ENTRY* get_dispatch_entry(SOAP_FW* fw, WsXmlDocH doc) {
    SOAP_DISPATCH_ENTRY* dispatch = NULL;

    if ( fw->dispatcherProc ) {    	    
        dispatch = (SOAP_DISPATCH_ENTRY*) fw->dispatcherProc(fw->cntx, fw->dispatcherData, doc);
    }

    if ( dispatch == NULL ) {    	
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "Dispatcher Error");    	
        if ( (dispatch = do_get_dispatch_entry(fw, doc)) == NULL )
            wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Dispatcher not defined"); 
    } else { 
        dispatch->usageCount++;
    }

    return dispatch;
}


SOAP_DISPATCH_ENTRY* do_get_dispatch_entry(SOAP_FW* fw, WsXmlDocH doc)
{
    SOAP_DISPATCH_ENTRY* dispatch = NULL;
    if ( fw )
    {
        char* action = get_soap_header_value(fw, doc, XML_NS_ADDRESSING, WSA_ACTION);
        if ( action == NULL )
        {                      		
            //ch->FaultCodeType = WSA_FAULT_INVALID_MESSAGE_INFORMATION_HEADER;  
            //ch->FaultDetailType = SOAP_FAULT_DETAIL_HEADER_NOT_UNDERSTOOD;           		       	            
        } else {
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
                        if ( !strncmp(action, dispatchAction, strlen(dispatchAction)) ) {
                            dispatch->usageCount++;
                            break;
                        }
                    } else {                    	
                        if ( !strcmp(action, dispatchAction) )
                        {
                            dispatch->usageCount++;
                            break;
                        }
                    }
                }					
                dispatch = (SOAP_DISPATCH_ENTRY*)DL_GetNext(&dispatch->node);
            }

            soap_fw_unlock(fw);

            if ( dispatch == NULL )
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "No dispatch entry for %s", action); 
            soap_free(action);
        }
    }

    return dispatch;
}


SoapDispatchH wsman_dispatcher(WsContextH cntx, void* data, WsXmlDocH doc) {	
    SoapDispatchH disp = NULL;
    if ( doc == NULL ) {
        soap_free(data);
    } else {
        WsManDispatcherInfo* dispInfo = (WsManDispatcherInfo*)data;
        wsman_dispatcher_list(dispInfo->interfaces);
        WsDispatchEndPointInfo* ep = NULL;
        WsDispatchEndPointInfo* ep_custom = NULL;
        char* uri = wsman_get_resource_uri(cntx, doc);
        char* action = ws_addressing_get_action(cntx, doc);
        int i; 
        int resUriMatch = 0;

        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"Dispatcher: %s, %s", uri, action);
        if ( uri && action ) {
        	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"Dispatcher: %s, %s", uri, action);
            WsDispatchInterfaceInfo* r = NULL;  
            GList *node = dispInfo->interfaces;

            while(node != NULL)
            {            
                WsDispatchInterfaceInfo* interface = (WsDispatchInterfaceInfo*)node->data;
                if (interface->wsmanResourceUri == NULL) 
                {
                    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"Dispatcher: interface->actionUriBase: %s", interface->actionUriBase);
                    if (strstr(uri, interface->actionUriBase)) {
                        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Dispatcher interface match: %s", interface->actionUriBase);
                        r = interface;
                        resUriMatch = 1;
                        break;                    
                    }
                }
                wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG,"Dispatcher: uri: %s, interface->wsmanResourceUri: %s", uri, interface->wsmanResourceUri);

                if (interface->wsmanResourceUri && !strcmp(uri, interface->wsmanResourceUri) )
                {     
                    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Dispatcher interface match: %s", interface->wsmanResourceUri);
                    r = interface;
                    resUriMatch = 1;
                    break;                    
                }
                node = g_list_next (node);                            
            }

            if ( r != NULL )
            {            	
                wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Resource Uri match");
                char* ptr = action;
                if ( r->actionUriBase )
                {
                    int len = strlen(r->actionUriBase);
                    if ( !strncmp(action, r->actionUriBase, len) && action[len] == '/' )
                        ptr = &action[len + 1];
                }

                for(i = 0; r->endPoints[i].serviceEndPoint != NULL; i++)
                {
                    if ( r->endPoints[i].inAction != NULL && !strcmp(ptr, r->endPoints[i].inAction) )
                    {
                        ep = &r->endPoints[i];
                        break;
                    } else if (r->endPoints[i].inAction == NULL) {
                        // Just store it for later in case no match is found for above condition
                        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Action is null, storing ep");
                        ep_custom = &r->endPoints[i];
                    }
                    
                }
            }
        }

        if ( uri )
            ws_remove_context_val(cntx, WSM_RESOURCE_URI);

        if ( ep != NULL ) {
            for(i = 0; i < dispInfo->mapCount; i++)
            {
                if ( dispInfo->map[i].ep == ep )
                {
                    disp = dispInfo->map[i].disp;
                    break;
                }
            }
        } else {
            if (ep_custom != NULL) {
                for(i = 0; i < dispInfo->mapCount; i++)
                {
                    if ( dispInfo->map[i].ep == ep_custom )
                    {
                        disp = dispInfo->map[i].disp;
                        break;
                    }
                }
            }
            if (resUriMatch == 1 ) 
                wsman_debug (WSMAN_DEBUG_LEVEL_ERROR,"No endpoint found for this action");
        }
    }	
    return disp;
}


/** @} */


