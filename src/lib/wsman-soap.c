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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"

#include "wsman-dispatcher.h"
#include "wsman-xml-serializer.h"
#include "wsman-client.h"
#include "wsman-soap-envelope.h"
#include "wsman-faults.h"
#include "wsman-soap-message.h"


WsXmlNsData g_wsNsData[] =
{
    {XML_NS_SOAP_1_2, "s"},
    {XML_NS_ADDRESSING, "wsa"},
    {XML_NS_EVENTING, "wse"},
    {XML_NS_ENUMERATION, "wsen"},
    {XML_NS_SCHEMA_INSTANCE, "xsi"},
    {XML_NS_CIM_SCHEMA, "cim"},
    {XML_NS_WS_MAN_CAT, "cat"},
    {XML_NS_WSMAN_ID, "wsmid"},
    {XML_NS_XML_SCHEMA, "xs"},
    {XML_NS_WS_MAN, "wsman"},
    {XML_NS_CIM_BINDING, "wsmb"},
    {NULL, NULL}
};

WsManDialectData g_wsDialectData[] =
{
    {WSM_WQL_FILTER_DIALECT, "wql"},
    {WSM_SELECTOR_FILTER_DIALECT, "selector"},
    {NULL, NULL}
};



callback_t*
make_callback_entry( SoapServiceCallback proc, 
                          void* data, 
                          list_t* list_to_add)
{
    debug("make new callback entry");
    callback_t* entry = 
        (callback_t*)u_malloc(sizeof(callback_t));

    if ( entry )
    {
        lnode_init(&entry->node, data);
        entry->proc = proc;
        if ( list_to_add )
            list_append(list_to_add, &entry->node);
    }
    return entry;
}

static void free_hentry_func(hnode_t *n, void *arg)
{
    u_free(hnode_getkey(n));
    u_free(hnode_get(n));
    u_free(n);
}


void
ws_initialize_context( WsContextH hCntx, 
                       SoapH soap)
{
    WS_CONTEXT* cntx = (WS_CONTEXT*)hCntx;
    cntx->entries = hash_create(HASHCOUNT_T_MAX, 0, 0);
    hash_set_allocator(cntx->entries, NULL, free_hentry_func, NULL);
    cntx->last_get_name_idx = -1;
    cntx->owner = 1;
    cntx->soap = soap;
}

WsContextH 
ws_create_context(SoapH soap)
{
    WS_CONTEXT* cntx = (WS_CONTEXT*)u_zalloc(sizeof(WS_CONTEXT));
    if ( cntx ) {
        ws_initialize_context((WsContextH)cntx, soap);
    }
    return (WsContextH)cntx;
}

SoapH
	ws_soap_initialize() 
{	
	env_t* fw = (env_t*)u_zalloc(sizeof(env_t));
	if ( fw ) {
		//fw->dispatchList.listOwner = fw;
		fw->cntx = ws_create_context((SoapH)fw);    	
		fw->inboundFilterList = list_create(LISTCOUNT_T_MAX);
		fw->outboundFilterList = list_create(LISTCOUNT_T_MAX);
		fw->dispatchList = list_create(LISTCOUNT_T_MAX);
		fw->responseList = list_create(LISTCOUNT_T_MAX);
		fw->processedMsgIdList = list_create(LISTCOUNT_T_MAX);
		fw->WsSerializerAllocList = list_create(LISTCOUNT_T_MAX);
		u_init_lock(fw);
	}	
	ws_xml_parser_initialize((SoapH)fw, g_wsNsData);
	soap_add_filter((SoapH)fw, outbound_addressing_filter, NULL, 0);
	soap_add_filter((SoapH)fw, outbound_control_header_filter, NULL, 0);
	return (SoapH)fw;
}



/**
 * Calculate needed space for interface array with Endpoints
 * @param interfaces List of interfaces
 * @return Needed size of WsManDispatcherInfo
 */
static int calculate_map_count(list_t *interfaces)
{
    int count = 0;
    int j;

    lnode_t *node = list_first(interfaces);
    while (node)
    {
        WsDispatchInterfaceInfo *interface = (WsDispatchInterfaceInfo *)node->list_data;
        for(j = 0; interface->endPoints[j].serviceEndPoint != NULL; j++)
            count++;
        node = list_next(interfaces, node);	
    }

    return (list_count(interfaces) * sizeof(WsManDispatcherInfo)) 
        + ( count * sizeof(DispatchToEpMap) );
}

/**
 * Register Dispatcher
 * @param cntx Context
 * @param proc Dispatcher Callback
 * @param data Callback data
 */
static void ws_register_dispatcher(WsContextH cntx, DispatcherCallback proc, void* data)
{
    env_t* soap = (env_t*)ws_context_get_runtime(cntx);   
    if ( soap) {
        soap->dispatcherProc = proc;
        soap->dispatcherData = data;
    }
    return;
}


WsContextH 
ws_create_runtime (list_t *interfaces)
{
    SoapH soap = ws_soap_initialize();
    if (soap && interfaces != NULL ) 
    {
        int size = calculate_map_count(interfaces);
        WsManDispatcherInfo* dispInfo = (WsManDispatcherInfo*)u_zalloc(size);
        if ( dispInfo == NULL ) {
            u_free(soap);
            soap = NULL;
        } else {     
            debug( "Registering %d plugins", (int )list_count(interfaces) );	
            dispInfo->interfaceCount = list_count(interfaces);
            dispInfo->interfaces = interfaces;
            lnode_t *node = list_first(interfaces);        	
            while(node != NULL)
            {                    	
                if ( wsman_register_interface(((env_t*)soap)->cntx, 
                            ( WsDispatchInterfaceInfo*) node->list_data,
                            dispInfo) != 0 )
                {
                    error( "Interface registeration failed");
                    u_free(dispInfo);
                    soap = NULL;                   
                }                                               
                node = list_next(interfaces, node);
            }                            
            if ( soap ) {
                ws_register_dispatcher(((env_t*)soap)->cntx, wsman_dispatcher, dispInfo);
            }
        }		
    }
    return ((env_t*)soap)->cntx;
}




/**
 * Register Dispatcher Interfaces
 * @param cntx WS-Man Context
 * @param wsInterface Interface
 * @param dispinfo Dispatcher
 */
int wsman_register_interface(WsContextH cntx, WsDispatchInterfaceInfo* wsInterface,
        WsManDispatcherInfo* dispInfo)
{
    int retVal = 0;
    int i; 

    WsDispatchEndPointInfo* ep = wsInterface->endPoints;
    for(i = 0;  ep[i].serviceEndPoint != NULL; i++)
    {    	
        if ( (retVal = wsman_register_endpoint(cntx, 
                        wsInterface,                         
                        &ep[i],
                        dispInfo)) != 0 )
            break;
    }
    return retVal;
}



/*
 * Register Endpoint
 * @param cntx Context
 * @param wsInterface Interface
 * @param ep Endpoint
 * @param dispInfo Dispatcher information
 */
int wsman_register_endpoint(WsContextH cntx, WsDispatchInterfaceInfo* wsInterface,
        WsDispatchEndPointInfo* ep, WsManDispatcherInfo* dispInfo)
{
    debug("Registering Endpoint: %s", ep->inAction);

    SoapDispatchH disp = NULL;
    unsigned long flags = SOAP_CUSTOM_DISPATCHER;
    SoapServiceCallback callbackProc = NULL;
    SoapH soap = ws_context_get_runtime(cntx);
    char* action = NULL;

    switch( (ep->flags & WS_DISP_TYPE_MASK) )
    {

    case WS_DISP_TYPE_IDENTIFY:
        debug("Registering endpoint for Identify");
        action = ep->inAction;
        callbackProc = wsman_identify_stub;
        break;
    case WS_DISP_TYPE_ENUMERATE:
        debug("Registering endpoint for Enumerate");
        action = ep->inAction;
        callbackProc = wsenum_enumerate_stub;
        break;
    case WS_DISP_TYPE_RELEASE:
        debug("Registering endpoint for Release");
        action = ep->inAction;
        callbackProc = wsenum_release_stub;
        break;
    case WS_DISP_TYPE_PULL:
        debug("Registering endpoint for Pull");
        action = ep->inAction;
        callbackProc = wsenum_pull_stub;
        break; 
    case WS_DISP_TYPE_PULL_RAW:
        debug("Registering endpoint for Pull Raw");
        action = ep->inAction;
        callbackProc = wsenum_pull_raw_stub;
        break;         
    case WS_DISP_TYPE_GET:
        debug("Registering endpoint for Get");
        action = ep->inAction;
        callbackProc = ws_transfer_get_stub;      
        break;
    case WS_DISP_TYPE_GET_RAW:
        debug("Registering endpoint for Get Raw");
        action = ep->inAction;
        callbackProc = (SoapServiceCallback)ep->serviceEndPoint;   
        break;        
    case WS_DISP_TYPE_PUT_RAW:
        debug("Registering endpoint for Put Raw");
        action = ep->inAction;
        callbackProc = (SoapServiceCallback)ep->serviceEndPoint;   
        break;        
    case WS_DISP_TYPE_PUT:
        debug("Registering endpoint for Put");
        action = ep->inAction;
        callbackProc = ws_transfer_put_stub;
        break;

    case WS_DISP_TYPE_RAW_DOC:
        action = ep->inAction;
        callbackProc = (SoapServiceCallback)ep->serviceEndPoint;
        break;
        
    case WS_DISP_TYPE_CUSTOM_METHOD:
        debug("Registering endpoint for custom method");
        action = ep->inAction;
        callbackProc = (SoapServiceCallback)ep->serviceEndPoint;   
        break;        

    case WS_DISP_TYPE_PRIVATE:
        debug("Registering endpoint for private EndPoint");
        action = ep->inAction;
        callbackProc = (SoapServiceCallback)ep->serviceEndPoint;   
        break;        

    default:
        debug("unknown dispatch type %lu", ep->flags & WS_DISP_TYPE_MASK);
        break;
    }

    if ( callbackProc != NULL &&
            (disp = soap_create_dispatch(soap, action, NULL, NULL, callbackProc, ep, flags)) )
    {

        dispInfo->map[dispInfo->mapCount].ep = ep;	        
        dispInfo->map[dispInfo->mapCount].disp = disp;        
        dispInfo->mapCount++;

        soap_start_dispatch(disp);
    }

    if ( action && action != ep->inAction ) {
        u_free(action);
    }

    return (disp == NULL);
}


WsEnumerateInfo*
get_enum_info( WsContextH cntx,
               WsXmlDocH doc, 
               char* cntxName, 
               int cntxNameLen,
               char* op, 
               char** enumIdPtr)
{
    WsEnumerateInfo* enumInfo = NULL;
    char* enumId = NULL;
    WsXmlNodeH node = ws_xml_get_soap_body(doc);

    if ( node && (node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, op)) )
    {
        node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
        if ( node ) {
            enumId = ws_xml_get_node_text(node);
            if ( enumIdPtr != NULL ) {
                *enumIdPtr = enumId;
            }
        }
    }
    debug("enum context: %s", enumId );

    if ( enumId != NULL )
    {
        strcpy(cntxName, WSFW_ENUM_PREFIX);
        strncpy(&cntxName[sizeof(WSFW_ENUM_PREFIX) - 1],
                enumId,
                cntxNameLen - sizeof(WSFW_ENUM_PREFIX));
        debug("enum context: %s", cntxName );
        enumInfo = (WsEnumerateInfo*)ws_get_context_val(cntx, cntxName, NULL);
        if (!enumInfo)
            error("enumInfo is null");
    }
    return enumInfo;
}



int 
wsman_identify_stub(SoapOpH op, 
                    void* appData) 
{
    debug( "Identify called");
    WsmanStatus *status = u_zalloc(sizeof(WsmanStatus *) );
    SoapH soap = soap_get_op_soap(op);
    WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));    

    WsDispatchEndPointInfo* info = (WsDispatchEndPointInfo*)appData;
    XmlSerializerInfo* typeInfo = info->serializationInfo;
    WsEndPointGet endPoint = (WsEndPointGet)info->serviceEndPoint;

    void* data;
    WsXmlDocH doc = NULL;

    if ( (data = endPoint(cntx, status)) == NULL ) {
        error( "Identify Fault");
        // FIXME
        doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), WSMAN_INTERNAL_ERROR, -1, NULL);
    } else {
        doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL);
        ws_serialize(cntx, ws_xml_get_soap_body(doc), data, typeInfo, 
                NULL, (char*)info->data, (char*)info->data, 1); 
        ws_serializer_free_mem(cntx, data, typeInfo);
    }

    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
    } else { 
        error( "Response doc invalid");
    }

    ws_serializer_free_all(cntx);
    ws_destroy_context(cntx);
    u_free(status);

    return 0;
}


int ws_transfer_put_stub(SoapOpH op, void* appData)
{
    WsmanStatus status;
    SoapH soap = soap_get_op_soap(op);	
    WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
    WsDispatchEndPointInfo* info = (WsDispatchEndPointInfo*)appData;
    XmlSerializerInfo* typeInfo = info->serializationInfo;
    WsEndPointPut endPoint = (WsEndPointPut)info->serviceEndPoint;

    WsXmlDocH _doc = soap_get_op_doc(op, 1);
    WsXmlNodeH _body = ws_xml_get_soap_body(_doc);
    WsXmlNodeH _r = ws_xml_get_child(_body, 0 , NULL, NULL);
    

    void *data = ws_deserialize(cntx, 
            ws_xml_get_soap_body(soap_get_op_doc(op, 1)), 
            typeInfo, ws_xml_get_node_local_name(_r),
            (char*)info->data, (char*)info->data, 0, 0); 

    int retVal = 0;
    WsXmlDocH doc = NULL;
    void* outData = NULL;

    if ( (retVal = endPoint(cntx, data, &outData, &status)) )
    {
        doc = wsman_generate_fault(cntx, _doc, status.fault_code, status.fault_detail_code, NULL);
    } 
    else 
    {
        doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL);
        if ( outData ) {
            ws_serialize(cntx, ws_xml_get_soap_body(doc), outData, 
                    typeInfo, NULL, (char*)info->data, (char*)info->data, 1); 
            ws_serializer_free_mem(cntx, outData, typeInfo);
        }
    }
    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
    }
    ws_serializer_free_all(cntx);
    return retVal;
}


/**
 * Enumeration Stub for processing enumeration requests
 * @param op SOAP pperation handler
 * @param appData Application data
 * @return status
 */
int
wsenum_enumerate_stub( SoapOpH op,
                   void* appData)
{
    WsXmlDocH doc = NULL;
    char cntxName[64] = WSFW_ENUM_PREFIX;
    char* enumId = &cntxName[sizeof(WSFW_ENUM_PREFIX) - 1];
    int retVal = 0;
    WsEnumerateInfo enumInfo;    
    WsmanStatus status;
    struct timeval tv;
    
    SoapH soap = soap_get_op_soap(op);  
    WsContextH soapCntx = ws_get_soap_context(soap);
      
    wsman_status_init(&status);
    
    WsDispatchEndPointInfo* ep = (WsDispatchEndPointInfo*)appData;
    WsEndPointEnumerate endPoint = (WsEndPointEnumerate)ep->serviceEndPoint;
        
    WsXmlDocH _doc = soap_get_op_doc(op, 1);
    WsContextH epcntx = ws_create_ep_context(soap, _doc);    
    
    memset(&enumInfo, 0, sizeof(enumInfo));    
    generate_uuid(enumId, sizeof(cntxName) - sizeof(WSFW_ENUM_PREFIX), 1);
      
    gettimeofday(&tv, NULL);
    enumInfo.timeStamp = tv.tv_sec * 10000000 + tv.tv_usec;    

    if ( endPoint && 
        ( retVal = endPoint(epcntx, &enumInfo, &status)) ) 
    {
        doc = wsman_generate_fault(epcntx, _doc, 
            status.fault_code, status.fault_detail_code, NULL);
    } 
    else 
    {              
        if ( enumInfo.pullResultPtr ) {
            doc = enumInfo.pullResultPtr;
            enumInfo.index++;
        } else {
            doc = ws_create_response_envelope(epcntx, _doc, NULL);
        }

        if ( doc )
        {
            WsXmlNodeH resp_node;
            wsman_set_estimated_total(_doc, doc, &enumInfo);
            WsXmlNodeH body = ws_xml_get_soap_body(doc);
            if ( enumInfo.pullResultPtr == NULL) {
                resp_node = ws_xml_add_child(body, XML_NS_ENUMERATION, WSENUM_ENUMERATE_RESP, NULL);
            } else {
                resp_node = ws_xml_get_child(body, 0, XML_NS_ENUMERATION, WSENUM_ENUMERATE_RESP);
            }

            if (enumInfo.index == enumInfo.totalItems) {
                ws_serialize_str(epcntx, resp_node, NULL, XML_NS_WS_MAN , WSENUM_END_OF_SEQUENCE);          
            } else {
                ws_serialize_str(epcntx, resp_node, enumId, 
                        XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
                ws_set_context_val(soapCntx, cntxName, &enumInfo, sizeof(enumInfo), 0);  
            }
        }
    }


    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
    }    
    ws_serializer_free_all(epcntx);
    ws_destroy_context(epcntx);    
    return retVal;
}


int
wsenum_release_stub( SoapOpH op,
                 void* appData)
{
    char cntxName[64];
    int retVal = 0;
    WsXmlDocH doc = NULL;
    WsmanStatus status;
    
    
    SoapH soap = soap_get_op_soap(op);
    WsContextH soapCntx = ws_get_soap_context(soap);
    WsDispatchEndPointInfo* ep = (WsDispatchEndPointInfo*)appData;
    WsEndPointRelease endPoint = (WsEndPointRelease)ep->serviceEndPoint;

    wsman_status_init(&status);
    WsXmlDocH _doc = soap_get_op_doc(op, 1);
    
    WsEnumerateInfo* enumInfo = get_enum_info(soapCntx, _doc, cntxName,
            sizeof(cntxName), WSENUM_RELEASE, NULL);

    if ( enumInfo == NULL ) 
    {
        doc = wsman_generate_fault(soapCntx, _doc, 
            WSEN_INVALID_ENUMERATION_CONTEXT, -1, NULL);
    } else {
        if ( endPoint && (retVal = endPoint(soapCntx, enumInfo, &status)) )
        {            
            error( "endPoint error");        		
            doc = wsman_generate_fault(soapCntx, _doc, 
                    WSMAN_INTERNAL_ERROR, OWSMAN_DETAIL_ENDPOINT_ERROR, NULL);   	            
        } else {
            doc = ws_create_response_envelope(soapCntx, _doc, NULL);
            debug("Releasing context: %s", cntxName);
            ws_remove_context_val(soapCntx, cntxName); 
        }
    }        
    if ( doc ) {
        soap_set_op_doc(op, doc, 0);        
    }

    return retVal;
}

int wsenum_pull_stub(SoapOpH op, void* appData)
{
    
    WsmanStatus status;
    SoapH soap = soap_get_op_soap(op);
    WsContextH soapCntx = ws_get_soap_context(soap);

    wsman_status_init(&status);    

    WsDispatchEndPointInfo* ep = (WsDispatchEndPointInfo*)appData;
    XmlSerializerInfo* typeInfo = ep->serializationInfo;
    WsEndPointPull endPoint = (WsEndPointPull)ep->serviceEndPoint;
    char cntxName[64];
    int retVal = 0;
    WsXmlDocH doc = NULL;
    char* enumId = NULL;
    
    WsXmlDocH _doc = soap_get_op_doc(op, 1);

    WsEnumerateInfo* enumInfo = get_enum_info(soapCntx, _doc,
            cntxName, sizeof(cntxName), WSENUM_PULL, &enumId);

    if ( enumInfo == NULL ) {
        doc = wsman_generate_fault(soapCntx, _doc, 
                WSEN_INVALID_ENUMERATION_CONTEXT, -1, NULL); 
    } else {
        if ( (retVal = endPoint(ws_create_ep_context(soap, _doc), enumInfo, &status)) ) {             
            doc = wsman_generate_fault(soapCntx, _doc, 
                status.fault_code, status.fault_detail_code, NULL);
            ws_remove_context_val(soapCntx, cntxName); 	
        } else {
            enumInfo->index++;
            if ( (doc = ws_create_response_envelope(soapCntx, _doc, NULL)) )
            {
                wsman_set_estimated_total(_doc, doc, enumInfo);
                WsXmlNodeH node = ws_xml_add_child(ws_xml_get_soap_body(doc), 
                        XML_NS_ENUMERATION, WSENUM_PULL_RESP, NULL);
                if ( node != NULL )
                {
                    if ( enumInfo->pullResultPtr )
                    {                       
                        WsXmlNodeH itemsNode = ws_xml_add_child(node, 
                                XML_NS_ENUMERATION, WSENUM_ITEMS, NULL);                            
                        ws_serialize(soapCntx, itemsNode, enumInfo->pullResultPtr, 
                                typeInfo, ep->respName, (char*)ep->data, (char*)ep->data, 1);                       
                        if ( enumId ) 
                        {
                            ws_serialize_str(soapCntx, node, enumId, 
                                    XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
                        }
                        ws_serializer_free_mem(soapCntx, enumInfo->pullResultPtr, typeInfo);
                    } else {
                        ws_serialize_str(soapCntx, 
                                node, NULL, XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE); 
                    }
                }
            }
        }
    }

    if ( doc )
    {
        soap_set_op_doc(op, doc, 0);
        ws_xml_destroy_doc(doc);
    }

    return retVal;
}

int 
wsenum_pull_raw_stub( SoapOpH op,
                  void* appData)
{
    WsmanStatus status;       
   
    WsXmlDocH doc = NULL;
    SoapH soap = soap_get_op_soap(op);
    WsContextH soapCntx = ws_get_soap_context(soap);
    WsDispatchEndPointInfo* ep = (WsDispatchEndPointInfo*)appData;
    wsman_status_init(&status);

    WsEndPointPull endPoint = (WsEndPointPull)ep->serviceEndPoint;
    char cntxName[64];
    int retVal = 0;
    char* enumId = NULL;
    WsXmlDocH _doc = soap_get_op_doc(op, 1);

    WsEnumerateInfo* enumInfo = get_enum_info(soapCntx, _doc, cntxName,
            sizeof(cntxName), WSENUM_PULL, &enumId);

    if ( enumInfo == NULL ) {        
        error( "Invalid enumeration context...");
        doc = wsman_generate_fault(soapCntx, _doc, 
            WSEN_INVALID_ENUMERATION_CONTEXT, -1, NULL);
    } else {
        if ( (retVal = endPoint(ws_create_ep_context(soap, _doc), enumInfo, &status)) ) 
        {             
            doc = wsman_generate_fault(soapCntx, _doc, 
                status.fault_code, status.fault_detail_code, NULL);
            ws_remove_context_val(soapCntx, cntxName); 	
        } else {
            enumInfo->index++;
            if ( enumInfo->pullResultPtr )
            {
                doc = 	enumInfo->pullResultPtr;
                wsman_set_estimated_total(_doc, doc, enumInfo);
                
                WsXmlNodeH body =   ws_xml_get_soap_body(doc);
                WsXmlNodeH response = ws_xml_get_child(body, 0 , XML_NS_ENUMERATION,WSENUM_PULL_RESP);
                
                if (enumInfo->index == enumInfo->totalItems) 
                {
                    ws_serialize_str(soapCntx, response, NULL, XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE);          
                }
                else if ( enumId ) {
                    ws_serialize_str(soapCntx, response, enumId, 
                            XML_NS_ENUMERATION, WSENUM_ENUMERATION_CONTEXT);
                }
            }
        }
    }
    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
    } else {
        error( "doc is null");
    }    
  
    return retVal;
}

int ws_transfer_get_stub(SoapOpH op, void* appData)
{
    WsmanStatus status;
    
    wsman_status_init(&status);    
    SoapH soap = soap_get_op_soap(op);
    WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));    

    WsDispatchEndPointInfo* info = (WsDispatchEndPointInfo*)appData;
    XmlSerializerInfo* typeInfo = info->serializationInfo;
    WsEndPointGet endPoint = (WsEndPointGet)info->serviceEndPoint;

    void* data;
    WsXmlDocH doc = NULL;

    if ( (data = endPoint(cntx, &status)) == NULL ) {
        warning( "Transfer Get fault");
        doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), 
            WSMAN_INVALID_SELECTORS, -1, NULL);
    } else {
        debug( "Creating Response doc");
        doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL);

        ws_serialize(cntx, ws_xml_get_soap_body(doc), data, typeInfo, 
                NULL, (char*)info->data, (char*)info->data, 1); 
        ws_serializer_free_mem(cntx, data, typeInfo);
    }

    if ( doc ) {
        debug( "Setting operation document");
        soap_set_op_doc(op, doc, 0);
    } 
    else { 
        warning( "Response doc invalid");
    }

    ws_serializer_free_all(cntx);
    ws_destroy_context(cntx);
    return 0;
}

WsContextH ws_get_soap_context(SoapH soap)
{
    return ((env_t*)soap)->cntx;
}


/*
void destroy_context_entry(WS_CONTEXT_ENTRY* entry)
{
    if ( (entry->options & WS_CONTEXT_FREE_DATA) != 0 )
        u_free(entry->node->list_data);
    u_free(entry->name);
    u_free(entry);
}
*/


void ws_clear_context_entries(WsContextH hCntx)
{
    if ( !hCntx ) {
        return;
    }
    hash_t* h = ((WS_CONTEXT*)hCntx)->entries;
    hash_free(h);
}

int
ws_remove_context_val( WsContextH hCntx, 
                       char* name)
{
    int retVal = 1;
    WS_CONTEXT* cntx = (WS_CONTEXT*)hCntx;
    if ( cntx && name )
    {
        u_lock(cntx->soap);
        hnode_t *hn = hash_lookup(cntx->entries, name);
        if (hn) {
            debug("Found context entry: %s", name);
            hash_delete_free(cntx->entries, hn);
            retVal = 0;
        }
        u_unlock(cntx->soap);
    }
    return retVal;
}

int
set_context_val( WsContextH hCntx, 
                 char* name, 
                 void* val, 
                 int size, 
                 int no_dup, 
                 unsigned long type)
{
    debug( "Setting context value: %s", name);
    int retVal = 1;
    WS_CONTEXT* cntx = (WS_CONTEXT*)hCntx;
    if ( cntx && name )
    {
        void* ptr = val;

        if ( !no_dup )
        {
            if ( val && (ptr = u_malloc(size)) )
            {
                memcpy(ptr, val, size);
            }
        }
        if ( ptr || val == NULL )
        {
            u_lock(cntx->soap);
            ws_remove_context_val(hCntx, name);
            if ( create_context_entry(cntx->entries, name, ptr) ) {
                retVal = 0;
            }
            u_unlock(cntx->soap);
        }
    } 
    else 
    {
        error( "error setting context value.");
    }
    return retVal;
}


int 
ws_set_context_val( WsContextH hCntx,
                    char* name, 
                    void* val, 
                    int size, 
                    int no_dup)
{
    debug("setting context value");
    int retVal = set_context_val(hCntx, name, val, size, no_dup, WS_CONTEXT_TYPE_BLOB);
    return retVal;
}



int
ws_set_context_ulong_val(WsContextH cntx, 
                         char* name, 
                         unsigned long val)
{
    int retVal = set_context_val(cntx, name, &val, sizeof(unsigned long), 0, 
            WS_CONTEXT_TYPE_ULONG); 
    return retVal;
}


int
ws_set_context_xml_doc_val( WsContextH cntx, 
                            char* name, 
                            WsXmlDocH val)
{
    int retVal = set_context_val(cntx, name, (void*)val, 0, 1, WS_CONTEXT_TYPE_XMLDOC); 
    return retVal;
}

WsContextH
ws_create_ep_context(SoapH soap,
                     WsXmlDocH doc)
{
    WsContextH cntx = ws_create_context(soap);
    if ( cntx ) 
        ws_set_context_xml_doc_val(cntx, WSFW_INDOC, doc);
    return cntx;
}


int
ws_destroy_context(WsContextH hCntx)
{
    int retVal = 1;
    WS_CONTEXT* cntx = (WS_CONTEXT*)hCntx;
    if ( cntx && cntx->owner ) {
        ws_clear_context_entries(hCntx);
        u_free(cntx);
        retVal = 0;
    }
    return retVal;
}


hnode_t*
create_context_entry(hash_t* h, 
                     char* name, 
                     void* val)
{
    char *key = u_strdup(name);
    hnode_t *hn = hnode_create(val);
    hash_insert(h, hn , (void *)key);
    return hn;
}

SoapH ws_context_get_runtime(WsContextH hCntx)
{
    SoapH soap = NULL;
    WS_CONTEXT* cntx = (WS_CONTEXT*)hCntx;
    if ( cntx )
        soap = cntx->soap;
    return soap;
}

void* get_context_val(WsContextH hCntx, char* name)
{
    char* val = NULL;
    WS_CONTEXT* cntx = (WS_CONTEXT*)hCntx;
    if ( cntx && name )
    {
        u_lock(cntx->soap);
        if (cntx->entries) {
            hnode_t *hn = hash_lookup(cntx->entries, name);
            if (hn)
                val = hnode_get(hn);
        }
        u_unlock(cntx->soap);
    }
    return val;
}


void* ws_get_context_val(WsContextH cntx, char* name, int* size)
{
    return get_context_val(cntx, name);
}


unsigned long
ws_get_context_ulong_val( WsContextH cntx,
                          char* name)
{
    void* ptr = get_context_val(cntx, name);
    if ( ptr != NULL )
        return *((unsigned long*)ptr);
    return 0;
}


SoapOpH soap_create_op( SoapH soap, 
                        char* inboundAction, 
                        char* outboundAction,// optional
                        char* role, 
                        SoapServiceCallback callbackProc, 
                        void* callbackData,
                        unsigned long flags, 
                        unsigned long timeout)
{
    dispatch_t* disp = NULL;
    op_t* entry = NULL;

    if ( (disp = (dispatch_t*)soap_create_dispatch(soap, 
                    inboundAction, outboundAction, NULL, // reserved, must be NULL
                    callbackProc, callbackData, flags)) != NULL )
    {
        entry = create_op_entry((env_t*)soap, disp, NULL, timeout);
    }
    return (SoapOpH)entry;
}



/**
 * Get Operation Document
 * @param op Operation Handle
 * @param inbound Direction flag
 * @return XML Document
 */
WsXmlDocH
soap_get_op_doc( SoapOpH op,
                 int inbound)
{
    WsXmlDocH doc = NULL;
    if ( op )
    {
        op_t* e = (op_t*)op;
        doc = (!inbound) ? e->out_doc : e->in_doc;
    }        

    return doc;
}

WsXmlDocH
soap_detach_op_doc( SoapOpH op,
                    int inbound)
{
    WsXmlDocH doc = NULL;
    if ( op ) {
        op_t* e = (op_t*)op;
        if ( !inbound ) {
            doc = e->out_doc;
            e->out_doc = NULL;
        } else {
            doc = e->in_doc;
            e->in_doc = NULL;
        }
    }

    return doc;
}

int
soap_set_op_doc( SoapOpH op,
                 WsXmlDocH doc,
                 int inbound)
{
    int retVal = 1;
    if ( op ) {
        op_t* e = (op_t*)op;
        if ( !inbound ) 
            e->out_doc = doc;
        else
            e->in_doc = doc;
        retVal = 0;
    }
    return retVal;
}

char*
soap_get_op_action( SoapOpH op,
                    int inbound)
{
    char* action = NULL;
    if ( op )
    {
        op_t* e = (op_t*)op;
        action = (!inbound) ? e->dispatch->outboundAction : 
            e->dispatch->inboundAction;
    }

    return action;
}

void
soap_set_op_action( SoapOpH op,
                    char* action,
                    int inbound)
{
    if ( op && action )
    {
        op_t* e = (op_t*)op;

        if ( !inbound )
        {
            u_free(e->dispatch->outboundAction);
            e->dispatch->outboundAction = u_str_clone(action);
        }
        else
        {
            u_free(e->dispatch->inboundAction);
            e->dispatch->inboundAction =  u_str_clone(action);
        }
    }
}

unsigned long 
soap_get_op_flags(SoapOpH op)
{
    if ( op )
    {
        return ((op_t*)op)->dispatch->flags;
    }
    return 0;
}

SoapH 
soap_get_op_soap(SoapOpH op)
{
    if ( op )
        return (SoapH)((op_t*)op)->dispatch->fw;

    return NULL;
}

void 
soap_destroy_op(SoapOpH op)
{
    destroy_op_entry((op_t*)op);
}


op_t*
create_op_entry( env_t* fw, 
                 dispatch_t* dispatch, 
                 WsmanMessage  *data, 
                 unsigned long timeout)
{
    op_t* entry = (op_t*)u_zalloc(sizeof(op_t));
    if ( entry ) {
        entry->timeoutTicks = timeout;
        entry->dispatch = dispatch;
        entry->cntx = ws_create_context((SoapH)fw);       
        entry->data = data;
        entry->processed_headers = list_create(LISTCOUNT_T_MAX);
    }
    return entry;
}


void
destroy_op_entry(op_t* entry)
{    
    debug("destroy op");
    if ( !entry )
    {
        debug("nothing to destroy...");
        return;
    }
    
    env_t* fw = entry->dispatch->fw;
    if ( fw ) {
        u_lock(fw);
    }

    if ( list_contains(fw->dispatchList, &entry->dispatch->node)) {
        list_delete(fw->dispatchList, &entry->dispatch->node);
    }

    unlink_response_entry(fw, entry);

    if ( fw ) {
        u_unlock(fw);
    }

    destroy_dispatch_entry(entry->dispatch);
    ws_destroy_context(entry->cntx);
    list_destroy_nodes(entry->processed_headers);
    list_destroy(entry->processed_headers);
    
    u_free(entry);
    
}

void 
destroy_dispatch_entry(dispatch_t* entry)
{
	int usageCount;
	
	if ( !entry )
		return;

	u_lock(entry->fw);
	entry->usageCount--;
	usageCount = entry->usageCount;
	if ( !usageCount && list_contains(entry->fw->dispatchList, &entry->node)) 
	{
		lnode_t *n = list_delete(entry->fw->dispatchList, &entry->node);
		lnode_destroy(n);
	}

	u_unlock(entry->fw);

	if ( !usageCount && entry->inboundFilterList && entry->outboundFilterList)
	{
		list_destroy_nodes(entry->inboundFilterList);
		list_destroy(entry->inboundFilterList);
		list_destroy_nodes(entry->outboundFilterList);
		list_destroy(entry->outboundFilterList);

		u_free(entry->inboundAction);
		u_free(entry->outboundAction);

		u_free(entry);
	}

}

WsXmlDocH
ws_get_context_xml_doc_val( WsContextH cntx,
                            char* name)
{
    return (WsXmlDocH)get_context_val(cntx, name);
}


void
add_response_entry(env_t* fw, op_t* op)
{
    debug( "Adding Response Entry");
    u_lock(fw);
    lnode_t *n = lnode_create( op);
    list_append(fw->responseList, n);
    u_unlock(fw);
}




void 
soap_destroy_fw( SoapH soap)
{
	env_t *fw = (env_t *)soap;
    if ( fw->dispatcherProc )
        fw->dispatcherProc(fw->cntx, fw->dispatcherData, NULL);

	if (fw->dispatchList)
	{
		while( !list_isempty(fw->dispatchList) )
		{		
			destroy_dispatch_entry((dispatch_t*)list_first(fw->dispatchList));				
		}
		list_destroy(fw->dispatchList);
	}

    while( !list_isempty(fw->processedMsgIdList) )
    {
        lnode_t* node = list_del_first(fw->processedMsgIdList);
        u_free(node->list_data);
        lnode_destroy(node);
    }
	list_destroy(fw->processedMsgIdList);
	
    while( !list_isempty(fw->responseList) )
    {
        lnode_t* node = list_del_first(fw->responseList);
        op_t* entry = (op_t*)node->list_data;
        destroy_op_entry(entry);
        lnode_destroy(node);
    }
	list_destroy(fw->responseList);

    list_destroy_nodes(fw->inboundFilterList);
	list_destroy(fw->inboundFilterList);
	
    list_destroy_nodes(fw->outboundFilterList);
	list_destroy(fw->outboundFilterList);
	
	list_destroy_nodes(fw->WsSerializerAllocList);
	list_destroy(fw->WsSerializerAllocList);
	

    ws_xml_parser_destroy((SoapH)soap);  
 
    ws_destroy_context(fw->cntx);
    //soap_destroy_lock(fw);
    u_free(soap);

    return;
}


void 
wsman_status_init(WsmanStatus* s)
{
    s->fault_code = 0;
    s->fault_detail_code = 0;
    s->fault_msg = NULL;
}

int
wsman_check_status( WsmanStatus *s)
{
    return s->fault_code;
}


