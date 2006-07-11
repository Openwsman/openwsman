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

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <CimClientLib/cmci.h>
#include <CimClientLib/native.h>
#include "ws_utilities.h"



#include "ws_errors.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"

#include "sfcc-interface.h"
#include "wsman-debug.h"


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
   ws_xml_add_child_format(refparam, XML_NS_WS_MAN , "ResourceUri" , "%s/%s", XML_NS_CIM_CLASS, (char *)classname->hdl);
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
            ws_xml_add_node_attr(nilnode, XML_NS_SCHEMA_INSTANCE, "nil", "true");
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
                    if (!ws_xml_add_node_attr(nilnode, XML_NS_SCHEMA_INSTANCE, "nil", "true"))
                        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "setting nil prop failed");
                }
                */
                if (valuestr) free (valuestr);
            }
        } else {
            WsXmlNodeH nilnode = ws_xml_add_child(node, resourceUri, name , NULL);
            ws_xml_add_node_attr(nilnode, XML_NS_SCHEMA_INSTANCE, "nil", "true");
            
        }
    }
}

void cim_getElementAt(WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode, char *resourceUri) {

    CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
    CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);
    instance2xml(data.value.inst, itemsNode, resourceUri);
    return;
}

void cim_getEprAt(WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode, char *resourceUri) {

    CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
    CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);

    CMPIInstance *instance = data.value.inst;
    CMPIObjectPath * objectpath = instance->ft->getObjectPath(instance, NULL);
    cim_add_epr(itemsNode, resourceUri, objectpath);
    CMRelease(objectpath);
    return;
}

void cim_getEprObjAt(WsEnumerateInfo* enumInfo, WsXmlNodeH itemsNode, char *resourceUri) {

    CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
    CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);

    CMPIInstance *instance = data.value.inst;
    CMPIObjectPath * objectpath = instance->ft->getObjectPath(instance, NULL);
    WsXmlNodeH item = ws_xml_add_child(itemsNode, XML_NS_WS_MAN, WSM_ITEM , NULL);
    cim_add_epr(item, resourceUri, objectpath);
    instance2xml(instance, item, resourceUri);

    CMRelease(objectpath);
    return;
}

void instance2xml( CMPIInstance *instance, WsXmlNodeH body, char *resourceUri)
{   
   CMPIObjectPath * objectpath = instance->ft->getObjectPath(instance, NULL);
   CMPIString * namespace = objectpath->ft->getNameSpace(objectpath, NULL);
   CMPIString * classname = objectpath->ft->getClassName(objectpath, NULL);

   int numproperties = instance->ft->getPropertyCount(instance, NULL);
   int i;
    
   char *className = resourceUri + sizeof(XML_NS_CIM_CLASS);

   WsXmlNodeH r = ws_xml_add_child(body, NULL, className , NULL);
   if (!ws_xml_ns_add(r, resourceUri, "p" ))
        wsman_debug (WSMAN_DEBUG_LEVEL_ERROR, "namespace failed: %s", resourceUri);
   if (!ws_xml_ns_add(r, XML_NS_SCHEMA_INSTANCE, "xsi" ))
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

#ifndef DMTF_WSMAN_SPEC_1
   add_cim_location(r, resourceUri, objectpath );
#endif

   if (classname) CMRelease(classname);
   if (namespace) CMRelease(namespace);
   if (objectpath) CMRelease(objectpath);
}


void cim_add_epr_details(WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath) {
   int numkeys = objectpath->ft->getKeyCount(objectpath, NULL);
   char *cv = NULL;
   int i;
    
   ws_xml_add_child(resource , XML_NS_ADDRESSING , "Address" , WSA_TO_ANONYMOUS);
   
   WsXmlNodeH refparam = ws_xml_add_child(resource, XML_NS_ADDRESSING , "ReferenceParameters" , NULL);
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

void cim_add_epr( WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath)
{
   WsXmlNodeH epr = ws_xml_add_child(resource, XML_NS_ADDRESSING, WSA_EPR , NULL);
   cim_add_epr_details(epr, resourceUri, objectpath );
   return;
}

void add_cim_location ( WsXmlNodeH resource , char *resourceUri,  CMPIObjectPath * objectpath)
{
   WsXmlNodeH cimLocation = ws_xml_add_child(resource, XML_NS_CIM_SCHEMA, "Location" , NULL);
   cim_add_epr_details(cimLocation, resourceUri, objectpath );
   return;
}

void cim_connect_to_cimom( CimClientInfo *cimclient, char *cim_host, char *cim_host_userid, 
        char *cim_host_passwd, WsmanStatus *status)
{

    CMPIStatus sfcc_status;
    cimclient->cc = cmciConnect(cim_host, NULL, DEFAULT_HTTP_CIMOM_PORT,
            cim_host_userid, cim_host_passwd, &sfcc_status);
    if (cimclient->cc == NULL) 
        wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Connection to CIMOM failed: %s", (char *)sfcc_status.msg->hdl);		
   //cim_to_wsman_status(sfcc_status, status);
   return;
}

CMPIConstClass * cim_get_class (CMCIClient *cc, char *class_name, WsmanStatus *status) 
{
    CMPIObjectPath * objectpath;    
    CMPIConstClass * class;
    CMPIStatus sfcc_status;

    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);

    class = cc->ft->getClass(cc, objectpath, 0, NULL, &sfcc_status);

    /* Print the results */
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "getClass() rc=%d, msg=%s",
            sfcc_status.rc, (sfcc_status.msg)? (char *)sfcc_status.msg->hdl : NULL);
    cim_to_wsman_status(sfcc_status, status);
    return class;            
}

void cim_invoke_method (CMCIClient *cc, char *class_name, 
        GList *keys, char *method, WsXmlNodeH r,  WsmanStatus *status) 
{
    CMPIObjectPath * objectpath;    
    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);
    CMPIArgs * argsin;
    CMPIStatus sfcc_status;

    GList *node = keys;
    while (node) 
    {    	
        WsSelectorInfo* selector = ( WsSelectorInfo*) node->data;
        wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Adding key: %s", selector->key);
        CMAddKey(objectpath, selector->key, selector->val, CMPI_chars);    	
        node = g_list_next (node);
    }
    argsin = newCMPIArgs(NULL);
    CMPIData data = cc->ft->invokeMethod( cc, objectpath, method, argsin, NULL, &sfcc_status);
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "invokeMethod() rc=%d, msg=%s",
            sfcc_status.rc, (sfcc_status.msg)? (char *)sfcc_status.msg->hdl : NULL);

    if (sfcc_status.rc == 0 )
        property2xml( data, "ReturnCode" , r, NULL);
    cim_to_wsman_status(sfcc_status, status);
    if (objectpath) CMRelease(objectpath);
    if (argsin) CMRelease(argsin);
    return;
}


void cim_get_instance (CMCIClient *cc, char *resourceUri, GList *keys, WsXmlNodeH body,
        WsmanStatus *status) 
{
    CMPIInstance * instance;
    CMPIObjectPath * objectpath;    
    CMPIStatus sfcc_status;

    char *class_name = resourceUri + sizeof(XML_NS_CIM_CLASS);
    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);

    GList *node = keys;
    while (node) 
    {    	
        WsSelectorInfo* selector = ( WsSelectorInfo*) node->data;
        wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Adding key: %s", selector->key);
        CMAddKey(objectpath, selector->key, selector->val, CMPI_chars);    	
        node = g_list_next (node);
    }
    instance = cc->ft->getInstance(cc, objectpath, CMPI_FLAG_DeepInheritance, NULL, &sfcc_status);
    if (instance)
        instance2xml(instance, body, resourceUri);

    /* Print the results */
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "getInstance() rc=%d, msg=%s",
            sfcc_status.rc, (sfcc_status.msg)? (char *)sfcc_status.msg->hdl : NULL);
    cim_to_wsman_status(sfcc_status, status);
    if (objectpath) CMRelease(objectpath);
    if (instance) CMRelease(instance);

}


CMPIInstance * cim_get_instance_raw (CMCIClient *cc, char *class_name, GList *keys ) 
{
    CMPIInstance * instance;
    CMPIObjectPath * objectpath;    
    CMPIStatus status;

    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);

    GList *node = keys;
    while (node) 
    {    	
        WsSelectorInfo* selector = ( WsSelectorInfo*) node->data;
        wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Adding key: %s", selector->key);
        CMAddKey(objectpath, selector->key, selector->val, CMPI_chars);    	
        node = g_list_next (node);
    }

    instance = cc->ft->getInstance(cc, objectpath, 0, NULL, &status);

    /* Print the results */
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "getInstance() rc=%d, msg=%s",
            status.rc, (status.msg)? (char *)status.msg->hdl : NULL);

    return instance;            
}
CMPIArray * cim_enum_instances_raw (CMCIClient *cc, char *class_name ) 
{
	
    CMPIObjectPath * objectpath;    
    CMPIStatus status;
    CMPIEnumeration * enumeration;
    
    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);
       
    enumeration = cc->ft->enumInstances(cc, objectpath, 0, NULL, &status);
    /* Print the results */
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "enumInstances() rc=%d, msg=%s",
            status.rc, (status.msg)? (char *)status.msg->hdl : NULL);
    
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "toArray() rc=%d, msg=%s",
            status.rc, (status.msg)? (char *)status.msg->hdl : NULL);
                  
    if (status.rc) {
	wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "CMCIClient enumInstances() failed");
	return NULL;
    }
    CMPIArray * enumArr =  enumeration->ft->toArray(enumeration, &status ); 
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Total enumeration items: %lu", 
    		enumArr->ft->getSize(enumArr, NULL ));
    if (objectpath) CMRelease(objectpath);
    if (enumeration) CMRelease(enumeration);
    return enumArr;           
    
}


void cim_to_wsman_status(CMPIStatus sfcc_status, WsmanStatus *status) {

    switch (sfcc_status.rc) {
    case CMPI_RC_OK:
        status->rc = WSMAN_RC_OK;
        break;
    case CMPI_RC_ERR_INVALID_CLASS:
        status->rc = WSA_FAULT_DESTINATION_UNREACHABLE;
        break;
    case CMPI_RC_ERR_FAILED:
        status->rc = WSMAN_FAULT_INTERNAL_ERROR;
        break;
    case CMPI_RC_ERR_METHOD_NOT_FOUND:
        status->rc = WSA_FAULT_ACTION_NOT_SUPPORTED;
        break;
    default:
        status->rc = WSMAN_FAULT_UNKNOWN;
    }
    /*
    if (sfcc_status.msg) {
        status->msg = (char *)soap_alloc(strlen(sfcc_status.msg->hdl ), 0 );
        status->msg = strndup(sfcc_status.msg->hdl, strlen(sfcc_status.msg->hdl ));
    }
    */
}

void cim_enum_instances (CMCIClient *cc, char *class_name , WsEnumerateInfo* enumInfo, WsmanStatus *status) 
{
    CMPIObjectPath * objectpath;    
    CMPIEnumeration * enumeration;
    CMPIStatus sfcc_status;
    //CMPIStatus enum_status;

    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);
    enumeration = cc->ft->enumInstances(cc, objectpath, CMPI_FLAG_IncludeClassOrigin, NULL, &sfcc_status);
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "enumInstances() rc=%d, msg=%s", sfcc_status.rc, (sfcc_status.msg)? (char *)sfcc_status.msg->hdl : NULL);

    if (sfcc_status.rc) {
        wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "CMCIClient enumInstances() failed");
        cim_to_wsman_status(sfcc_status, status);
        return;
    }
    CMPIArray * enumArr =  enumeration->ft->toArray(enumeration, NULL ); 

    cim_to_wsman_status(sfcc_status, status);
    if (!enumArr) {
        return;
    }

    
    enumInfo->totalItems = cim_enum_totalItems(enumArr);
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Total items: %lu", enumInfo->totalItems );
    enumInfo->enumResults = enumArr;
    enumInfo->appEnumContext = enumeration;

    if (objectpath) CMRelease(objectpath);
    //if (enumeration) CMRelease(enumeration);
    return;           
}

void cim_release_enum_context( WsEnumerateInfo* enumInfo ) {
    if (enumInfo->appEnumContext) {
        CMPIEnumeration * enumeration = (CMPIEnumeration *)enumInfo->appEnumContext;
        if (enumeration) CMRelease(enumeration);
    }
}

CMPIArray *cim_enum_instancenames (CMCIClient *cc, char *class_name , WsmanStatus *status) 
{

    CMPIStatus sfcc_status;
    CMPIObjectPath * objectpath;    
    CMPIEnumeration * enumeration;

    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);

    enumeration = cc->ft->enumInstanceNames(cc, objectpath, &sfcc_status);
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "enumInstanceNames() rc=%d, msg=%s",
            sfcc_status.rc, (sfcc_status.msg)? (char *)sfcc_status.msg->hdl : NULL);

    if (sfcc_status.rc) {
        wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "CMCIClient enumInstanceNames() failed");
        cim_to_wsman_status(sfcc_status, status);
        return NULL;
    }
    CMPIArray * enumArr =  enumeration->ft->toArray(enumeration, NULL ); 
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Total enumeration items: %lu", 
            enumArr->ft->getSize(enumArr, NULL ));
    cim_to_wsman_status(sfcc_status, status);
    return enumArr;           

}


CMPICount cim_enum_totalItems (CMPIArray * enumArr) 
{
    return enumArr->ft->getSize(enumArr, NULL );
}


char* cim_get_property(CMPIInstance *instance, char *property)
{

    CMPIStatus status;
    CMPIData data = instance->ft->getProperty(instance, property, &status);	
    char *valuestr = NULL;
    if (CMIsArray(data)) 
    {
        return NULL;
    } else {
        if ( data.type != CMPI_null && data.state != CMPI_nullValue) {

            if (data.type ==  CMPI_ref) {
                //refpoint =  ws_xml_add_child(node, resourceUri, name , NULL);
                //path2xml(refpoint, resourceUri,  &data.value);
            } else {
                valuestr = value2Chars(data.type, &data.value);
            }
        }
    }
    return valuestr;
}

char *cim_get_keyvalue(CMPIObjectPath *objpath, char *keyname)
{
    CMPIStatus status;
    char *valuestr;
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Get key property from objpath");

    if (!objpath) {
        wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "objpath is NULL");
        return "";
    }

    CMPIData data = objpath->ft->getKey(objpath, keyname, &status);	

    if (status.rc || CMIsArray(data)) {
        return "";
    } else {
        valuestr = value2Chars(data.type, &data.value);
        return valuestr;
    }	
}

void cim_get_enum_items(WsContextH cntx, WsXmlNodeH node, WsEnumerateInfo* enumInfo, char *namespace, int max) {
    WsXmlNodeH itemsNode;
    char *resourceUri = wsman_remove_query_string(wsman_get_resource_uri(cntx, NULL));
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Resource Uri: %s", resourceUri); 

    if ( node != NULL ) {
        itemsNode = ws_xml_add_child(node, namespace, WSENUM_ITEMS, NULL);     	
        wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Total items: %lu", enumInfo->totalItems );
        if (max > 0 ) {
            while(max > 0 && enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems) {
                if ( ( enumInfo->flags & FLAG_ENUMERATION_ENUM_EPR) == FLAG_ENUMERATION_ENUM_EPR )
                    cim_getEprAt(enumInfo, itemsNode, resourceUri);
                else if ( ( enumInfo->flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) == FLAG_ENUMERATION_ENUM_OBJ_AND_EPR )
                    cim_getEprObjAt(enumInfo, itemsNode, resourceUri);
                else
                    cim_getElementAt(enumInfo, itemsNode, resourceUri);
                enumInfo->index++;
                max--;
            }
            enumInfo->index--;
        } else {
            if ( enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems ) {
                if ( ( enumInfo->flags & FLAG_ENUMERATION_ENUM_EPR) == FLAG_ENUMERATION_ENUM_EPR )
                    cim_getEprAt(enumInfo, itemsNode, resourceUri);
                else if ( ( enumInfo->flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) == FLAG_ENUMERATION_ENUM_OBJ_AND_EPR )
                    cim_getEprObjAt(enumInfo, itemsNode, resourceUri);
                else
                    cim_getElementAt(enumInfo, itemsNode, resourceUri);
            }
        }
    }   
}


