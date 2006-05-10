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
#include "interface_utils.h"
#include "cim_namespace_data.h"

#include <cmci.h>
#include <cimcdt.h>


void property2xml( CMPIData data, char *name , WsXmlNodeH node, char *resourceUri);
void instance2xml( CMPIInstance *instance , WsXmlNodeH body, char  *resourceUri);
extern char *value2Chars(CMPIType type, CMPIValue * value);
void class2xml( CMPIConstClass * class, WsXmlNodeH node, char *resourceUri );
void path2xml(  WsXmlNodeH node, char *resourceUri ,  CMPIValue *val);
void add_cim_location ( WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath);


void path2xml(  WsXmlNodeH node, char *resourceUri, CMPIValue * val)
{
   CMPIObjectPath *objectpath = val->ref;
   CMPIString * namespace = objectpath->ft->getNameSpace(objectpath, NULL);
   CMPIString * classname = objectpath->ft->getClassName(objectpath, NULL);
   int numkeys = objectpath->ft->getKeyCount(objectpath, NULL);
   int i;
   char *cv = NULL;

   ws_xml_add_child(node, XML_NS_ADDRESSING , "Address" , WSA_TO_ANONYMOUS);
   WsXmlNodeH refparam = ws_xml_add_child(node, XML_NS_ADDRESSING , "ReferenceParameters" , NULL);
   ws_xml_add_child_format(refparam, XML_NS_WS_MAN , "ResourceUri" , "%s/%s", XML_NS_CIM_V2_9, (char *)classname->hdl);
   WsXmlNodeH wsman_selector_set = ws_xml_add_child(refparam, XML_NS_WS_MAN , "SelectorSet" , NULL);

   if (numkeys) {
      for (i=0; i<numkeys; i++) {
         CMPIString * keyname;
         CMPIData data = objectpath->ft->getKeyAt(objectpath, i, &keyname, NULL);
         WsXmlNodeH s = ws_xml_add_child(wsman_selector_set, XML_NS_WS_MAN , "Selector" , (char *)value2Chars(data.type, &data.value));
         ws_xml_add_node_attr(s, NULL , "Name" , (char *)keyname->hdl);
         if(cv) free(cv);
         if(keyname) CMRelease(keyname);
      }
   }

   if (classname) CMRelease(classname);
   if (namespace) CMRelease(namespace);
}

void class2xml( CMPIConstClass * class, WsXmlNodeH node, char *resourceUri )
{
   CMPIString * classname = class->ft->getClassName(class, NULL);
   int numproperties = class->ft->getPropertyCount(class, NULL);
   int i;
   char *cv = NULL;


   WsXmlNodeH r = NULL;
   if (classname && classname->hdl) 
        r = ws_xml_add_child(node, NULL,  (char *)classname->hdl , NULL);
   
   if (!ws_xml_ns_add(r, resourceUri, "p" ))
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "namespace failed: %s", resourceUri);

   if (numproperties) {
      for (i=0; i<numproperties; i++) {
         CMPIString * propertyname;
         CMPIData data = class->ft->getPropertyAt(class, i, &propertyname, NULL);
         if (data.state == 0) {
            property2xml( data, (char *)propertyname->hdl , r, resourceUri);
            if(cv) free(cv);
         }
         else 
            property2xml( data, (char *)propertyname->hdl , r, resourceUri);

         if (propertyname) CMRelease(propertyname);
      }
   }  

   if (classname) CMRelease(classname);
}

void property2xml( CMPIData data, char *name , WsXmlNodeH node, char *resourceUri)
{
    char *valuestr = NULL;
    if (CMIsArray(data)) 
    {
        if ( data.type != CMPI_null && data.state != CMPI_nullValue) {
            WsXmlNodeH nilnode = ws_xml_add_child(node, resourceUri, name , NULL);
            ws_xml_add_node_attr(nilnode, XML_NS_XML_SCHEMA_INSTANCE, "nil", "true");
            return;
        }
        CMPIArray *arr   = data.value.array;
        CMPIType  eletyp = data.type & ~CMPI_ARRAY;
        int j, n;
        n = CMGetArrayCount(arr, NULL);
        for (j = 0; j < n; ++j) {
            CMPIData ele = CMGetArrayElementAt(arr, j, NULL);
            valuestr = value2Chars(eletyp, &ele.value);
            ws_xml_add_child(node, resourceUri, name , valuestr);
            free (valuestr);
        }
    } else {
        if ( data.type != CMPI_null && data.state != CMPI_nullValue) {
            WsXmlNodeH nilnode = NULL, refpoint = NULL;

            if (data.type ==  CMPI_ref) {
                refpoint =  ws_xml_add_child(node, resourceUri, name , NULL);
                path2xml(refpoint, resourceUri,  &data.value);
            } else {
                valuestr = value2Chars(data.type, &data.value);
                nilnode = ws_xml_add_child(node, resourceUri, name , valuestr);

                /*
                if (strcmp(valuestr, "") == 0) {
                    if (!ws_xml_add_node_attr(nilnode, XML_NS_XML_SCHEMA_INSTANCE, "nil", "true"))
                        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "setting nil prop failed");
                }
                */
                if (valuestr) free (valuestr);
            }
        } else {
            WsXmlNodeH nilnode = ws_xml_add_child(node, resourceUri, name , NULL);
            ws_xml_add_node_attr(nilnode, XML_NS_XML_SCHEMA_INSTANCE, "nil", "true");
            
        }
    }
}

void instance2xml( CMPIInstance *instance, WsXmlNodeH body, char *resourceUri)
{   
   CMPIObjectPath * objectpath = instance->ft->getObjectPath(instance, NULL);
   CMPIString * objectpathname = objectpath->ft->toString(objectpath, NULL);
   CMPIString * namespace = objectpath->ft->getNameSpace(objectpath, NULL);
   CMPIString * classname = objectpath->ft->getClassName(objectpath, NULL);

   int numproperties = instance->ft->getPropertyCount(instance, NULL);
   wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "number of properties: %d", numproperties);
   int i;
    
   char *className = resourceUri + sizeof(XML_NS_CIM_V2_9);

   WsXmlNodeH r = ws_xml_add_child(body, NULL, className , NULL);
   if (!ws_xml_ns_add(r, resourceUri, "p" ))
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "namespace failed: %s", resourceUri);
   if (!ws_xml_ns_add(r, XML_NS_XML_SCHEMA_INSTANCE, "xsi" ))
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "namespace failed: %s", resourceUri);
       
   if (numproperties) 
   {
      for (i=0; i<numproperties; i++) {
         CMPIString * propertyname;
         CMPIData data = instance->ft->getPropertyAt(instance, i, &propertyname, NULL);
         property2xml( data, (char *)propertyname->hdl , r, resourceUri);
         CMRelease(propertyname);
      }
   }
   add_cim_location(r, resourceUri, objectpath );

   if (classname) CMRelease(classname);
   if (namespace) CMRelease(namespace);
   if (objectpathname) CMRelease(objectpathname);
   if (objectpath) CMRelease(objectpath);
}

void add_cim_location ( WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath)
{
   int numkeys = objectpath->ft->getKeyCount(objectpath, NULL);
   char *cv = NULL;
   int i;


   // cim:Location
   WsXmlNodeH cimLocation = ws_xml_add_child(resource, XML_NS_CIM_V2_9, "Location" , NULL);
   ws_xml_add_child(cimLocation, XML_NS_ADDRESSING , "Address" , WSA_TO_ANONYMOUS);
   
   WsXmlNodeH refparam = ws_xml_add_child(cimLocation, XML_NS_ADDRESSING , "ReferenceParameters" , NULL);
   ws_xml_add_child_format(refparam, XML_NS_WS_MAN , "ResourceUri" , "%s", resourceUri );
   WsXmlNodeH wsman_selector_set = ws_xml_add_child(refparam, XML_NS_WS_MAN , "SelectorSet" , NULL);

   if (numkeys) {
      for (i=0; i<numkeys; i++) {
         CMPIString * keyname;
         CMPIData data = objectpath->ft->getKeyAt(objectpath, i, &keyname, NULL);
         WsXmlNodeH s = NULL;
         if (data.type ==  CMPI_ref) {
             s = ws_xml_add_child(wsman_selector_set, XML_NS_WS_MAN , "Selector" , NULL);
             WsXmlNodeH epr =  ws_xml_add_child(s, XML_NS_ADDRESSING , "EndpointReference" , NULL);
             path2xml(epr, resourceUri,  &data.value);
         } else {
             char *valuestr = value2Chars(data.type, &data.value);
             s = ws_xml_add_child(wsman_selector_set, XML_NS_WS_MAN , "Selector" , valuestr);
             if (valuestr) free (valuestr);
         }
         ws_xml_add_node_attr(s, NULL , "Name" , (char *)keyname->hdl);


         if(cv) free(cv);
         if(keyname) CMRelease(keyname);
      }
   }
   return;
}

int  CimResource_Get_EP(SoapOpH op, void* appData, WsmanStatus * status )
{
    WsXmlDocH doc = NULL;
    CMPIStatus sfcc_status;
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Get Endpoint Called");

    SoapH soap = soap_get_op_soap(op);
    WsContextH cntx = ws_create_ep_context(soap, soap_get_op_doc(op, 1));
    GList *keys = wsman_get_selector_list(cntx, NULL);
    char *resourceUri = wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL));

    if (keys)
    {
        wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, 
                "Number of keys defined: %d", g_list_length(keys));
        CimClientInfo cimclient;
        cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL , &sfcc_status);
        if (!cimclient.cc)
            return 1;		
        char *className = resourceUri + sizeof(XML_NS_CIM_V2_9);
        CMPIInstance *instance = cim_get_instance(cimclient.cc, className, keys, &sfcc_status);
        
        if (instance) 
        {					
            if ( (doc = ws_create_response_envelope(cntx, soap_get_op_doc(op, 1), NULL)) )
            {    		
                WsXmlNodeH body = ws_xml_get_soap_body(doc);
                instance2xml(instance, body, resourceUri);
            }
        } else {
            doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), WSA_FAULT_DESTINATION_UNREACHABLE, -1);
        }
        if (instance) CMRelease(instance);
        if (cimclient.cc) CMRelease(cimclient.cc);
    } 
    else 
    {
        doc = wsman_generate_fault(cntx, soap_get_op_doc(op, 1), WSMAN_FAULT_INVALID_SELECTORS, -1);
    } 

    if ( doc )
    {
        soap_set_op_doc(op, doc, 0);
        soap_submit_op(op, soap_get_op_channel_id(op), NULL);
    } 
    else 
    {
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Invalid doc" );
    }

    return 0;

}


int CimResource_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo, WsmanStatus *status)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Enumerate Endpoint Called"); 
    CMPIStatus sfcc_status;
    char *resourceUri = wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL));
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Resource Uri: %s", resourceUri); 
    char *className = resourceUri + sizeof(XML_NS_CIM_V2_9);

    CimClientInfo cimclient;
    cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL , &sfcc_status);
    if (!cimclient.cc)
    {
        /*
        status->rc = sfcc_status.rc;
        status->msg = sfcc_status.msg->hdl;
        */
        return 1;
    }

    CMPIArray * enumArr = cim_enum_instances (cimclient.cc, className , &sfcc_status);
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "enumInstances() rc=%d, msg=%s",
            sfcc_status.rc, (sfcc_status.msg)? (char *)sfcc_status.msg->hdl : NULL);
    if (!enumArr) {
        switch (sfcc_status.rc) {
        case CMPI_RC_ERR_INVALID_CLASS:
            status->rc = WSA_FAULT_DESTINATION_UNREACHABLE;
            break;
        default:
            status->rc = WSMAN_FAULT_NO_DETAILS;
        }
        status->msg = strdup(sfcc_status.msg->hdl);
        return 1;
    }

    enumInfo->totalItems = cim_enum_totalItems(enumArr);
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Total items: %lu", enumInfo->totalItems );
    enumInfo->enumResults = enumArr;
    if (cimclient.cc) CMRelease(cimclient.cc);
    return 0;
}

int CimResource_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo, WsmanStatus *status)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Release Endpoint Called");      
    return 0;
}

int CimResource_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo, WsmanStatus *status)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Pull Endpoint Called");      
    WsXmlDocH doc = NULL;
    WsXmlNodeH itemsNode = NULL;

    char *resourceUri = wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL));
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Resource Uri: %s", resourceUri); 

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
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Total items: %lu", enumInfo->totalItems );
    if ( enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems ) 
    {
        CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
        CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Resource URI: %s", resourceUri);      
        //class2xml(class, itemsNode);
        instance2xml(data.value.inst, itemsNode, resourceUri);
    }
    else
        enumInfo->pullResultPtr = NULL;

    if (doc != NULL )
        enumInfo->pullResultPtr = doc;

    return 0;
}





