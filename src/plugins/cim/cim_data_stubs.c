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

#ifdef HAVE_CONFIG_H
#include "wsman_config.h"
#endif

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "u/libu.h"



#include "wsman-errors.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"

#include "wsman-soap-envelope.h"
#include "sfcc-interface.h"
#include "cim_data.h"


static CimClientInfo*
CimResource_Init(WsContextH cntx)
{
    char *_tmp = NULL;
	char *r = NULL;
    CimClientInfo *cimclient= (CimClientInfo *)u_zalloc(sizeof(CimClientInfo));

	wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL), &r);
	
    cimclient->cc = NULL;
    cimclient->namespaces = get_vendor_namespaces();
    cimclient->selectors = wsman_get_selector_list(cntx, NULL);
    cimclient->requested_class = wsman_get_class_name(cntx);
    cimclient->method = wsman_get_method_name(cntx);
    if (cimclient->selectors) {
        _tmp = cim_get_namespace_selector(cimclient->selectors);
    }
    if (_tmp)
        cimclient->cim_namespace = _tmp;
    else
        cimclient->cim_namespace = get_cim_namespace();
    cimclient->resource_uri = u_strdup(r);
    cimclient->method_args = wsman_get_method_args(cntx, r );
	// u_free(r);
    return cimclient;
}

static void
CimResource_destroy(CimClientInfo *cimclient)
{

    if (cimclient->resource_uri) u_free(cimclient->resource_uri);
    if (cimclient->method) u_free(cimclient->method);
    if (cimclient->requested_class) u_free(cimclient->requested_class);
    if (cimclient->method_args) {
        //hash_free(cimclient->method_args);       
    }
    if (cimclient->selectors) {
        hash_free(cimclient->selectors);        
    }
    u_free(cimclient);
    return;
}

int
CimResource_Get_EP( SoapOpH op,
                    void* appData )
{
    debug( "Get Endpoint Called");
    WsXmlDocH doc = NULL;
    WsmanStatus status;
    CimClientInfo *cimclient;

    wsman_status_init(&status);
    SoapH soap = soap_get_op_soap(op);
    WsXmlDocH in_doc = NULL;

    in_doc = soap_get_op_doc(op, 1);
    WsContextH cntx = ws_create_ep_context(soap, in_doc);
    cimclient = CimResource_Init(cntx);

    if ( (doc = ws_create_response_envelope(cntx, in_doc, NULL)) ) {    		
        WsXmlNodeH body = ws_xml_get_soap_body(doc);
        cim_get_instance_from_enum(cimclient, cntx, body, &status);
    }

    if (status.fault_code != 0) 
    {
        ws_xml_destroy_doc(doc);
        doc = wsman_generate_fault(cntx, in_doc, status.fault_code, status.fault_detail_code, NULL);
    }

    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
    } else {
        debug( "Invalid doc" );
    }
    
    CimResource_destroy(cimclient);
    ws_destroy_context(cntx);
    return 0;
}

int 
CimResource_Custom_EP( SoapOpH op, 
                       void* appData )
{
    debug( "Custom Method Endpoint Called");
    WsXmlDocH doc = NULL;
    CimClientInfo *cimclient;
    WsmanStatus status;
    WsXmlDocH in_doc = NULL;

    wsman_status_init(&status);
    SoapH soap = soap_get_op_soap(op);
    in_doc = soap_get_op_doc(op, 1); 
    WsContextH cntx = ws_create_ep_context(soap, in_doc);
    
    cimclient = CimResource_Init(cntx);

    if ( (doc = ws_create_response_envelope(cntx, in_doc, NULL)) ) {    		
        WsXmlNodeH body = ws_xml_get_soap_body(doc);
        cim_invoke_method(cimclient, cntx, body, &status);
    }

    if (status.fault_code != 0) {
        ws_xml_destroy_doc(doc);
        doc = wsman_generate_fault(cntx, in_doc, status.fault_code, status.fault_detail_code, NULL);
    }

    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
    } else {
        error( "Invalid doc" );
    }

    ws_destroy_context(cntx);
    CimResource_destroy(cimclient);

    return 0;
}




int
CimResource_Enumerate_EP( WsContextH cntx,
                          WsEnumerateInfo* enumInfo,
                          WsmanStatus *status)
{
    int max_elements = 0;
    WsXmlDocH doc;
    char *enum_mode;

    CimClientInfo *cimclient = CimResource_Init(cntx);
    cim_enum_instances (cimclient, enumInfo,  status);

    if (status && status->fault_code != 0) {
        goto err;
    }

    max_elements = wsman_is_optimization(cntx, NULL );
    enum_mode = wsman_get_enum_mode(cntx, NULL); 
    if (enum_mode)
        wsman_set_enum_mode(enum_mode, enumInfo);

    if (max_elements > 0)
    {
        doc = ws_create_response_envelope(cntx, ws_get_context_xml_doc_val(cntx, WSFW_INDOC), NULL);
        WsXmlNodeH node = ws_xml_add_child(ws_xml_get_soap_body(doc), XML_NS_ENUMERATION, 
            WSENUM_ENUMERATE_RESP , NULL);       
        cim_get_enum_items(cimclient, cntx, node, enumInfo, XML_NS_WS_MAN, max_elements);
        if (doc != NULL ) {
            enumInfo->pullResultPtr = doc;
            int index2 = enumInfo->index + 1;
            if (index2 == enumInfo->totalItems)  {
                cim_release_enum_context(enumInfo);
            }
        }
        else
            enumInfo->pullResultPtr = NULL;
    }
   
    ws_destroy_context(cntx);
    CimResource_destroy(cimclient);
    return 0;
err:
    return 1;
}

int CimResource_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo, WsmanStatus *status)
{
    debug( "Release Endpoint Called");      
    cim_release_enum_context(enumInfo);
    return 0;
}


int
CimResource_Pull_EP( WsContextH cntx,
                     WsEnumerateInfo* enumInfo,
                     WsmanStatus *status)
{
    debug( "Pull Endpoint Called");      
    WsXmlDocH doc = NULL;
    
    
    CimClientInfo *cimclient = CimResource_Init(cntx);
    
    doc = ws_create_response_envelope(cntx, ws_get_context_xml_doc_val(cntx, WSFW_INDOC), NULL);
    WsXmlNodeH pullnode = ws_xml_add_child(ws_xml_get_soap_body(doc), XML_NS_ENUMERATION, 
            WSENUM_PULL_RESP, NULL);       

    int max = wsen_get_max_elements(cntx, NULL);
    cim_get_enum_items(cimclient, cntx, pullnode, enumInfo, XML_NS_ENUMERATION,  max);
    
    if (doc != NULL )
        enumInfo->pullResultPtr = doc;
    else
        enumInfo->pullResultPtr = NULL;

    if ( ( enumInfo->index + 1 ) == enumInfo->totalItems) {
        cim_release_enum_context(enumInfo);
    }



    CimResource_destroy(cimclient);
    ws_destroy_context(cntx);
    return 0;
}



int
CimResource_Put_EP( SoapOpH op,
                    void* appData )
{
    debug( "Put Endpoint Called");
    WsXmlDocH doc = NULL;
    WsmanStatus status;

    SoapH soap = soap_get_op_soap(op);
    WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
    CimClientInfo *cimclient = CimResource_Init(cntx);

    if ( (doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL)) ) 
    { 
        WsXmlNodeH body = ws_xml_get_soap_body(doc);
        WsXmlNodeH in_body = ws_xml_get_soap_body(soap_get_op_doc(op, 1));
        cim_put_instance_from_enum(cimclient, cntx , in_body, body, &status);
    }

    if (wsman_check_status(&status) != 0) {
        ws_xml_destroy_doc(doc);
        doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), status.fault_code, status.fault_detail_code, NULL);
    }

    if ( doc ) {
        soap_set_op_doc(op, doc, 0);
    } else {
        debug( "Invalid doc" );
    }
    
    CimResource_destroy(cimclient);
    ws_destroy_context(cntx);
    return 0;
}


