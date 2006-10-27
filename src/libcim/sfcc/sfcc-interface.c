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
#include <CimClientLib/cmci.h>
#include <CimClientLib/native.h>
#include "u/libu.h"


#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>

#include "wsman-errors.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "sfcc-interface.h"
#include "cim-interface.h"



static void 
cim_add_args( CMPIArgs *argsin,
              hash_t *args)
{
    hscan_t hs;
    hnode_t *hn;
    hash_scan_begin(&hs, args);
    while ((hn = hash_scan_next(&hs))) {    	
        CMAddArg(argsin,  (char*) hnode_getkey(hn), (char*) hnode_get(hn) , CMPI_chars);  
    }
}


static char * 
cim_find_namespace_for_class ( CimClientInfo *client,
                               char *class) 
{
    char *ns = NULL;
    char *sub;
    hscan_t hs;
    hnode_t *hn;

    debug("%s", client->method );
    if (strstr(client->resource_uri , XML_NS_CIM_CLASS ) != NULL  && 
            ( strcmp(client->method, TRANSFER_GET) == 0 ||  strcmp(client->method, TRANSFER_PUT) == 0  )) {
        ns = u_strdup(client->resource_uri);
        return ns;
    }
    if ( class && client->namespaces) {
        hash_scan_begin(&hs, client->namespaces);
        while ((hn = hash_scan_next(&hs))) {    	
            if ( (sub = strstr(class,  (char*) hnode_getkey(hn)))) {
                ns = u_strdup_printf("%s/%s", (char*) hnode_get(hn), class);
                break;
            }
        }
    }
    if (!ns)
        ns = u_strdup_printf("%s/%s", XML_NS_CIM_CLASS, class);
    return ns;
}

char*
cim_get_namespace_selector(hash_t *keys) 
{
    char *cim_namespace= NULL;
    hnode_t *hn = hash_lookup(keys, (char *)CIM_NAMESPACE_SELECTOR );
    if (hn) {
        cim_namespace = (char *)hnode_get(hn);
        hash_delete(keys, hn);
        debug("CIM Namespace: %s", cim_namespace);
    }
    return cim_namespace;
}

static void 
cim_add_keys( CMPIObjectPath * objectpath,
              hash_t *keys)
{
    hscan_t hs;
    hnode_t *hn;
    hash_scan_begin(&hs, keys);
    while ((hn = hash_scan_next(&hs))) {    	
        CMAddKey(objectpath,  (char*) hnode_getkey(hn), (char*) hnode_get(hn) , CMPI_chars);  
    }
}

static int
cim_verify_keys( CMPIObjectPath * objectpath,
                 hash_t *keys,
                 WsmanStatus *statusP)
{
    debug("verify selectors");
    CMPIStatus rc;
    hscan_t hs;
    hnode_t *hn;
	int count;

	if (!keys) {
	  	count = 0;
	} else {
		count = (int)hash_count(keys);
	}
	
    if (CMGetKeyCount(objectpath, NULL) >  count ) 
    {
        statusP->fault_code = WSMAN_INVALID_SELECTORS;
        statusP->fault_detail_code = WSMAN_DETAIL_INSUFFICIENT_SELECTORS;
        goto cleanup;
    } else if (CMGetKeyCount(objectpath, NULL) < hash_count(keys) )  {
        statusP->fault_code = WSMAN_INVALID_SELECTORS;
        goto cleanup;
    }

    hash_scan_begin(&hs, keys);
    char *cv;
    while ((hn = hash_scan_next(&hs))) {    	
        CMPIData data = CMGetKey(objectpath, (char*) hnode_getkey(hn), &rc);
        if ( rc.rc != 0 ) { // key not found
            statusP->fault_code = WSMAN_INVALID_SELECTORS;
            statusP->fault_detail_code = WSMAN_DETAIL_UNEXPECTED_SELECTORS;
            break;
        }
        cv=value2Chars(data.type, &data.value);
        if (strcmp(cv, (char*) hnode_get(hn)) == 0 )  {
            statusP->fault_code = WSMAN_RC_OK;
            statusP->fault_detail_code = WSMAN_DETAIL_OK;
            u_free(cv);
        } else  {
            statusP->fault_code = WSA_DESTINATION_UNREACHABLE;
            statusP->fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
            u_free(cv);
            break;
        }
    }
cleanup:
    debug("fault: %d %d",  statusP->fault_code, statusP->fault_detail_code );
    return  statusP->fault_code;
}

/*
static int
cim_check_valid_keys(
        CMPIObjectPath * objectpath,
        hash_t *keys)
{
    CMPIStatus rc;
    hscan_t hs;
    hnode_t *hn;
    hash_scan_begin(&hs, keys);
    while ((hn = hash_scan_next(&hs))) {    	
        CMAddKey(objectpath,  (char*) hnode_getkey(hn), (char*) hnode_get(hn) , CMPI_chars);  
        CMGetKey(objectpath, (char*) hnode_getkey(hn), &rc);
        if ( rc.rc != 0 )
            break;
    }
    return  ( rc.rc != 0 )? -1: 0;
}
*/

static void
instance2xml( CimClientInfo *client,
              CMPIInstance* instance,
              WsXmlNodeH body)
{   
   int i;
   char *new_ns = NULL;
   CMPIObjectPath * objectpath = instance->ft->getObjectPath(instance, NULL);
   CMPIString * namespace = objectpath->ft->getNameSpace(objectpath, NULL);
   CMPIString * classname = objectpath->ft->getClassName(objectpath, NULL);

   int numproperties = instance->ft->getPropertyCount(instance, NULL);

   new_ns = cim_find_namespace_for_class(client, (char *)classname->hdl);
   char *final_class = u_strdup(strrchr(new_ns, '/') + 1);
   WsXmlNodeH r = ws_xml_add_child(body, NULL, final_class , NULL);

   u_free(final_class);
   
   WsXmlNsH ns = ws_xml_ns_add(r, new_ns, "p" );
   
   //FIXME
   xmlSetNs((xmlNodePtr) r, (xmlNsPtr) ns );

   if (!ws_xml_ns_add(r, XML_NS_SCHEMA_INSTANCE, "xsi" ))
        debug( "namespace failed: %s", client->resource_uri);
       
   if (numproperties) 
   {
      for (i=0; i<numproperties; i++) {
         CMPIString * propertyname;
         CMPIData data = instance->ft->getPropertyAt(instance, i, &propertyname, NULL);
         property2xml( data, (char *)propertyname->hdl , r, new_ns);
         CMRelease(propertyname);
      }
   }

#ifndef DMTF_WSMAN_SPEC_1
   add_cim_location(r, client->resource_uri, objectpath );
#endif

   if (classname) CMRelease(classname);
   if (namespace) CMRelease(namespace);
   if (objectpath) CMRelease(objectpath);
   if (new_ns) u_free(new_ns);
}
#if 0
static CMPIConstClass*
cim_get_class (CimClientInfo *client,
               char *class, 
               WsmanStatus *status) 
{
    CMPIObjectPath * objectpath;    
    CMPIConstClass * _class;
    CMPIStatus rc;
    CMCIClient* cc = cim_connect_to_cimom("localhost", NULL, NULL , status);
    if (!cc) {
        return NULL;
    }
    client->cc = (CMCIClient *)cc;

    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class,  NULL);
    _class = cc->ft->getClass(cc, objectpath, 0, NULL, &rc);

    /* Print the results */
    debug( "getClass() rc=%d, msg=%s",
            rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);
    cim_to_wsman_status(rc, status);
    return _class;            
}
#endif

static CMPIObjectPath *  
cim_get_op_from_enum( CimClientInfo *client,
                      WsmanStatus *statusP )
{
    CMPIStatus rc;
    int match = 0;
    CMPIEnumeration * enumeration;
    CMPIObjectPath * result_op = NULL;
    WsmanStatus statusPP;

    CMPIObjectPath * objectpath = newCMPIObjectPath(client->cim_namespace, client->requested_class, NULL);
    enumeration = ((CMCIClient *)client->cc)->ft->enumInstanceNames(client->cc, objectpath, &rc);
    if (rc.rc != 0 ) {
        debug( "rc=%d, msg=%s",
                rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);
        cim_to_wsman_status(rc, statusP);
        statusP->fault_detail_code = WSMAN_DETAIL_INVALID_RESOURCEURI;
        cim_to_wsman_status(rc, statusP);
        goto cleanup;
    }

    
    wsman_status_init(&statusPP);
    while (enumeration->ft->hasNext(enumeration, NULL)) 
    {
        CMPIData data = enumeration->ft->getNext(enumeration, NULL);
        CMPIObjectPath *op = CMClone(data.value.ref, NULL);
        if (cim_verify_keys(op, client->selectors, &statusPP) != 0 ) {
            if (op) CMRelease(op);
            continue;
        } else {
            result_op =  CMClone(data.value.ref, NULL);
            CMSetNameSpace(result_op, client->cim_namespace);
            match = 1;
            if (op) CMRelease(op);
            break;
        }
        if (op) CMRelease(op);
    }
    statusP->fault_code = statusPP.fault_code;
    statusP->fault_detail_code = statusPP.fault_detail_code;
    debug("fault: %d %d",  statusP->fault_code, statusP->fault_detail_code );

cleanup:
    if (objectpath) CMRelease(objectpath);
    if (enumeration) CMRelease(enumeration);
    if (match)
        return result_op;
    else
        return NULL;
}

void 
cim_enum_instances (CimClientInfo *client,
                    WsEnumerateInfo* enumInfo, 
                    WsmanStatus *status) 
{
    CMPIObjectPath * objectpath;    
    CMPIEnumeration * enumeration;
    CMPIStatus rc;

    objectpath = newCMPIObjectPath(client->cim_namespace, client->requested_class , NULL);
    CMCIClient* cc = cim_connect_to_cimom("localhost", NULL, NULL , status);
    if (!cc) {
        goto cleanup;
    }
    client->cc = (CMCIClient *)cc;

    enumeration = cc->ft->enumInstances(cc, objectpath, CMPI_FLAG_IncludeClassOrigin, NULL, &rc);
    
    debug( "enumInstances() rc=%d, msg=%s", rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);

    if (rc.rc) {
        debug( "CMCIClient enumInstances() failed");
        cim_to_wsman_status(rc, status);
        goto cleanup;
    }
    CMPIArray * enumArr =  enumeration->ft->toArray(enumeration, NULL ); 

    cim_to_wsman_status(rc, status);
    if (!enumArr) {
        goto cleanup;
    }

    enumInfo->totalItems = cim_enum_totalItems(enumArr);
    debug( "Total items: %d", enumInfo->totalItems );
    enumInfo->enumResults = enumArr;
    enumInfo->appEnumContext = enumeration;

cleanup:
    if (cc) CMRelease(cc);
    if (objectpath) CMRelease(objectpath);
    return;           
}


void
path2xml(WsXmlNodeH node,
         char *resourceUri,
         CMPIValue * val)
{
   int i;
   char *cv = NULL;

   CMPIObjectPath *objectpath = val->ref;
   CMPIString * namespace = objectpath->ft->getNameSpace(objectpath, NULL);
   CMPIString * classname = objectpath->ft->getClassName(objectpath, NULL);
   int numkeys = objectpath->ft->getKeyCount(objectpath, NULL);

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





void
class2xml( CMPIConstClass* class,
           WsXmlNodeH node,
           char *resourceUri )
{
   CMPIString * classname = class->ft->getClassName(class, NULL);
   int numproperties = class->ft->getPropertyCount(class, NULL);
   int i;
   char *cv = NULL;


   WsXmlNodeH r = NULL;
   if (classname && classname->hdl) 
        r = ws_xml_add_child(node, NULL,  (char *)classname->hdl , NULL);
   
   if (!ws_xml_ns_add(r, resourceUri, "p" ))
        debug( "namespace failed: %s", resourceUri);

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




void
xml2property( CMPIInstance *instance,
              CMPIData data,
              char *name,
              char *value )
{
    CMPIType type = data.type;

    if (type & CMPI_ARRAY) {
    }
    else if (type & CMPI_ENC) {

        switch (type) {
        case CMPI_instance:
            break;

        case CMPI_ref:
            break;

        case CMPI_args:
            break;

        case CMPI_filter:
            break;

        case CMPI_string:
        case CMPI_numericString:
        case CMPI_booleanString:
        case CMPI_dateTimeString:
        case CMPI_classNameString:
            CMSetProperty(instance, name, value, CMPI_chars);
            break;
        case CMPI_dateTime:
            break;
        }

    }
    else if (type & CMPI_SIMPLE) {

        int yes = 0;
        switch (type) {
        case CMPI_boolean:
            if (strcmp(value, "true") == 0 )
                yes = 1;
            CMSetProperty(instance, name, (CMPIValue *)&yes, CMPI_boolean);
            break;
        case CMPI_char16:
            CMSetProperty(instance, name, value, CMPI_chars);
             break;
        }

    }
    else if (type & CMPI_INTEGER) {

        unsigned long tmp;
        unsigned long long tmp_ll;
        int val;
        long val_l;
        long long val_ll;
        switch (type) {
        case CMPI_uint8:
            tmp = strtoul(value, NULL, 10);
            CMSetProperty(instance, name, (CMPIValue *)&tmp, type);
            break;
        case CMPI_sint8:
            val = atoi(value);
            CMSetProperty(instance, name, (CMPIValue *)&val, type);
            break;
        case CMPI_uint16:
            tmp = strtoul(value, NULL, 10);
            CMSetProperty(instance, name, (CMPIValue *)&tmp, type);
            break;
        case CMPI_sint16:
            val = atoi(value);
            CMSetProperty(instance, name, (CMPIValue *)&val, type);
            break;
        case CMPI_uint32:
            tmp = strtoul(value, NULL, 10);
            CMSetProperty(instance, name, (CMPIValue *)&tmp, type);
            break;
        case CMPI_sint32:
            val_l = atol(value);
            CMSetProperty(instance, name, (CMPIValue *)&val_l, type);
            break;
        case CMPI_uint64:
            tmp_ll = strtoull(value, NULL, 10);
            CMSetProperty(instance, name, (CMPIValue *)&tmp, type);
            break;
        case CMPI_sint64:
            val_ll = atoll(value);
            CMSetProperty(instance, name, (CMPIValue *)&val_ll, type);
            break;
        }

    }
    else if (type & CMPI_REAL) {

        switch (type) {
        case CMPI_real32:
            break;
        case CMPI_real64:
            break;
        }

    }

    /*
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

                if (valuestr) free (valuestr);
            }
        } else {
            WsXmlNodeH nilnode = ws_xml_add_child(node, resourceUri, name , NULL);
            ws_xml_add_node_attr(nilnode, XML_NS_SCHEMA_INSTANCE, "nil", "true");
            
        }
    }
    */
}

void
property2xml( CMPIData data,
              char *name,
              WsXmlNodeH node,
              char *resourceUri)
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
        if (arr != NULL) {
            n = CMGetArrayCount(arr, NULL);
            for (j = 0; j < n; ++j) {
                CMPIData ele = CMGetArrayElementAt(arr, j, NULL);
                valuestr = value2Chars(eletyp, &ele.value);
                ws_xml_add_child(node, resourceUri, name , valuestr);
                free (valuestr);
            }
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
                        debug( "setting nil prop failed");
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

void
cim_getElementAt(CimClientInfo *client,
                 WsEnumerateInfo* enumInfo,
                 WsXmlNodeH itemsNode) 
{
    CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
    CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);
    instance2xml(client, data.value.inst, itemsNode);
    return;
}

void
cim_getEprAt( CimClientInfo *client,
              WsEnumerateInfo* enumInfo, 
              WsXmlNodeH itemsNode)
{

    CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
    CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);

    CMPIInstance *instance = data.value.inst;
    CMPIObjectPath * objectpath = instance->ft->getObjectPath(instance, NULL);
    cim_add_epr(itemsNode, client->resource_uri, objectpath);
    CMRelease(objectpath);
    return;
}

void 
cim_getEprObjAt(CimClientInfo *client,
                WsEnumerateInfo* enumInfo,
                WsXmlNodeH itemsNode)
{
    CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
    CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);

    CMPIInstance *instance = data.value.inst;
    CMPIObjectPath * objectpath = instance->ft->getObjectPath(instance, NULL);
    WsXmlNodeH item = ws_xml_add_child(itemsNode, XML_NS_WS_MAN, WSM_ITEM , NULL);
    cim_add_epr(item, client->resource_uri, objectpath);
    instance2xml(client, instance, item);

    CMRelease(objectpath);
    return;
}


void
xml2instance( CMPIInstance *instance,
              WsXmlNodeH body,
              char *resourceUri)
{   
   int i;
   CMPIObjectPath * objectpath = instance->ft->getObjectPath(instance, NULL);
   CMPIString * namespace = objectpath->ft->getNameSpace(objectpath, NULL);
   CMPIString * classname = objectpath->ft->getClassName(objectpath, NULL);

   int numproperties = instance->ft->getPropertyCount(instance, NULL);
   debug( "properties: %d", numproperties);
   char *className = resourceUri + sizeof(XML_NS_CIM_CLASS);

   WsXmlNodeH r = ws_xml_get_child(body, 0, resourceUri, className);
       
   if (numproperties) 
   {
      for (i=0; i<numproperties; i++) {
         CMPIString * propertyname;
         CMPIData data = instance->ft->getPropertyAt(instance, i, &propertyname, NULL);
         debug( "property: %s", (char *)propertyname->hdl);
         WsXmlNodeH child  = ws_xml_get_child(r, 0, resourceUri, (char *)propertyname->hdl);
         char *value =  ws_xml_get_node_text(child);
         debug( "property value: %s", value);
         xml2property(instance, data, (char *)propertyname->hdl, value );
         CMRelease(propertyname);
      }
   }

   if (classname) CMRelease(classname);
   if (namespace) CMRelease(namespace);
   if (objectpath) CMRelease(objectpath);
}





void
cim_add_epr_details( WsXmlNodeH resource,
                     char *resourceUri,
                     CMPIObjectPath * objectpath) 
{
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

void
cim_add_epr( WsXmlNodeH resource,
             char *resourceUri,
             CMPIObjectPath* objectpath)
{
   WsXmlNodeH epr = ws_xml_add_child(resource, XML_NS_ADDRESSING, WSA_EPR , NULL);
   cim_add_epr_details(epr, resourceUri, objectpath );
   return;
}

void
add_cim_location ( WsXmlNodeH resource,
                   char *resourceUri,
                   CMPIObjectPath* objectpath)
{
   WsXmlNodeH cimLocation = ws_xml_add_child(resource, XML_NS_CIM_SCHEMA, "Location" , NULL);
   cim_add_epr_details(cimLocation, resourceUri, objectpath );
   return;
}

CMCIClient *
cim_connect_to_cimom( char *cim_host,
                      char *cim_host_userid, 
                      char *cim_host_passwd, 
                      WsmanStatus *status)
{

    CMPIStatus rc;
    CMCIClient *cimclient = cmciConnect(cim_host, NULL, DEFAULT_HTTP_CIMOM_PORT,
            cim_host_userid, cim_host_passwd, &rc);
    if (cimclient == NULL) 
        debug( "Connection to CIMOM failed: %s", (char *)rc.msg->hdl);		
   return cimclient;
}



void
cim_invoke_method (CimClientInfo *client, 
                   WsContextH cntx,
                   WsXmlNodeH body,  
                   WsmanStatus *status) 
{
    CMPIObjectPath * objectpath;    
    CMPIArgs *argsin, *argsout;
    CMPIStatus rc;
    CMCIClient *cc = cim_connect_to_cimom( "localhost", NULL, NULL , status);
    if (!cc) {
        return;
    }

    objectpath = newCMPIObjectPath(client->cim_namespace, client->requested_class, NULL);
    if (client->selectors && hash_count(client->selectors) > 0)
        cim_add_keys(objectpath, client->selectors);

    // FIXME, bug #27
    argsin = newCMPIArgs(NULL);
    
    if (client->method_args && hash_count(client->method_args))
        cim_add_args(argsin, client->method_args);

    argsout = newCMPIArgs(NULL);
    CMPIData data = cc->ft->invokeMethod( cc, objectpath,
            client->method, argsin, argsout, &rc);

    debug( "invokeMethod() rc=%d, msg=%s",
            rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);

    WsXmlNodeH method_node = ws_xml_add_empty_child_format(body, 
            client->resource_uri , "%s_OUTPUT", client->method);
    if (rc.rc == 0 ) 
        property2xml( data, "ReturnValue" , method_node, NULL);

    if (argsout) {
        int count = CMGetArgCount(argsout, NULL);
        int i = 0;
        for (i=0; i<count; i++) {
            CMPIString * argname;
            CMPIData data = CMGetArgAt(argsout, i, &argname, NULL);
            property2xml( data, (char *)argname->hdl , method_node, NULL );
            CMRelease(argname);
        }
    }

    cim_to_wsman_status(rc, status);
    if (objectpath) CMRelease(objectpath);
    if (argsin) CMRelease(argsin);
    if (argsout) CMRelease(argsout);
    if (cc) CMRelease(cc);
    return;
}

void
cim_get_instance_from_enum ( CimClientInfo *client,   
                             WsContextH cntx,
                             WsXmlNodeH body, 
                             WsmanStatus *status) 
{
    CMPIInstance * instance;
    CMPIObjectPath * objectpath;
    CMPIStatus rc;
    WsmanStatus statusP;
    CMCIClient * cc = cim_connect_to_cimom( "localhost", NULL, NULL , status);
    client->cc = cc;
    if (!cc) {
        goto cleanup;
    }
    wsman_status_init(&statusP);

    if ( (objectpath = cim_get_op_from_enum(client, &statusP )) != NULL ) 
    {
        instance = cc->ft->getInstance(cc, objectpath, CMPI_FLAG_IncludeClassOrigin , NULL, &rc);
        if (rc.rc == 0 ) {
            if (instance) 
                instance2xml(client, instance, body);
        } else {
            cim_to_wsman_status(rc, status);
        }
        debug( "rc=%d, msg=%s", rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);

        if (instance) CMRelease(instance);
    } else {
        status->fault_code = statusP.fault_code;
        status->fault_detail_code = statusP.fault_detail_code;
    }

    debug("fault: %d %d",  status->fault_code, status->fault_detail_code );

    if (objectpath) CMRelease(objectpath);
cleanup:
    if (cc) CMRelease(cc);
    return;
}


void 
cim_put_instance_from_enum (CimClientInfo *client, 
                            WsContextH cntx,
                            WsXmlNodeH in_body,
                            WsXmlNodeH body, 
                            WsmanStatus *status) 
{
    CMPIInstance * instance;
    CMPIObjectPath * objectpath;    
    CMPIStatus rc;
    WsmanStatus statusP;
    wsman_status_init(&statusP);
    client->cc = (CMCIClient *)cim_connect_to_cimom( "localhost", NULL, NULL , status);
    if (!client->cc) {
        return;
    }
    if ( (objectpath = cim_get_op_from_enum(client, &statusP )) != NULL ) 
    {
        instance = ((CMCIClient *)client->cc)->ft->getInstance(client->cc, objectpath, CMPI_FLAG_DeepInheritance, NULL, &rc);
        debug( "getInstance() rc=%d, msg=%s", rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);

        xml2instance(instance, in_body, client->resource_uri);

        rc = ((CMCIClient *)client->cc)->ft->setInstance(client->cc, objectpath, instance,  0, NULL );
        debug( "modifyInstance() rc=%d, msg=%s", rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);

        cim_to_wsman_status(rc, status);
        if (rc.rc == 0 ) {
            if (instance)
                instance2xml(client, instance, body);
        } 
        if (instance) CMRelease(instance);
    } else {
        status->fault_code = statusP.fault_code;
        status->fault_detail_code = statusP.fault_detail_code;
    }
    if (objectpath) CMRelease(objectpath);
    return;
}




void 
cim_get_instance (CimClientInfo *client, 
                  WsContextH cntx,
                  WsXmlNodeH body,
                  WsmanStatus *status) 
{
    CMPIInstance * instance;
    CMPIObjectPath * objectpath;    
    CMPIStatus rc;

    CMCIClient *cc = cim_connect_to_cimom( "localhost", NULL, NULL , status);
    if (!cc) {
        return;
    }

    objectpath = newCMPIObjectPath(client->cim_namespace, client->requested_class, NULL);

    cim_add_keys(objectpath, client->selectors);
    instance = cc->ft->getInstance(cc, objectpath, CMPI_FLAG_DeepInheritance, NULL, &rc);
    if (instance)
        instance2xml(client, instance, body);

    /* Print the results */
    debug( "getInstance() rc=%d, msg=%s",
            rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);
    cim_to_wsman_status(rc, status);
    if (objectpath) CMRelease(objectpath);
    if (instance) CMRelease(instance);

}


void 
cim_to_wsman_status(CMPIStatus rc, 
                    WsmanStatus *status)
{
    switch (rc.rc) {
    case CMPI_RC_OK:
        status->fault_code = WSMAN_RC_OK;
        break;
    case CMPI_RC_ERR_INVALID_CLASS:
        status->fault_code = WSA_DESTINATION_UNREACHABLE;
        break;
    case CMPI_RC_ERR_FAILED:
        if (strcmp((char *)rc.msg->hdl, "CURL error: 7") == 0)
            status->fault_code = WSA_DESTINATION_UNREACHABLE;
        else
            status->fault_code = WSMAN_INTERNAL_ERROR;
        break;
    case CMPI_RC_ERR_METHOD_NOT_FOUND:
        status->fault_code = WSA_ACTION_NOT_SUPPORTED;
       break;
    case CMPI_RC_ERR_NOT_FOUND:
        status->fault_code = WSA_DESTINATION_UNREACHABLE;
        break;
    case CMPI_RC_ERR_ACCESS_DENIED:
        status->fault_code = WSMAN_ACCESS_DENIED;
        break;
    case CMPI_RC_ERR_INVALID_NAMESPACE:
    case CMPI_RC_ERR_INVALID_PARAMETER:
    case CMPI_RC_ERR_NOT_SUPPORTED:
    case CMPI_RC_ERR_CLASS_HAS_CHILDREN:
    case CMPI_RC_ERR_CLASS_HAS_INSTANCES:
    case CMPI_RC_ERR_INVALID_SUPERCLASS:
    case CMPI_RC_ERR_ALREADY_EXISTS:
    case CMPI_RC_ERR_NO_SUCH_PROPERTY:
    case CMPI_RC_ERR_TYPE_MISMATCH:
    case CMPI_RC_ERR_QUERY_LANGUAGE_NOT_SUPPORTED:
    case CMPI_RC_ERR_INVALID_QUERY:
    case CMPI_RC_ERR_METHOD_NOT_AVAILABLE:
    case CMPI_RC_DO_NOT_UNLOAD:
    case CMPI_RC_NEVER_UNLOAD:
    case CMPI_RC_ERROR_SYSTEM:
    case CMPI_RC_ERROR:
    default:
        status->fault_code = WSMAN_UNKNOWN;
    }
    /*
    if (rc.msg) {
        status->msg = (char *)soap_alloc(strlen(rc.msg->hdl ), 0 );
        status->msg = strndup(rc.msg->hdl, strlen(rc.msg->hdl ));
    }
    */
}


void
cim_release_enum_context( WsEnumerateInfo* enumInfo ) 
{
    if (enumInfo->appEnumContext) {
        CMPIEnumeration * enumeration = (CMPIEnumeration *)enumInfo->appEnumContext;
        /*
        CMPIArray * array = (CMPIArray *)enumInfo->enumResults;
        if (array) CMRelease(array);
        */
        if (enumeration) CMRelease(enumeration);
    }
}

CMPIArray* 
cim_enum_instancenames (CimClientInfo *client, 
                        char *class_name ,
                        WsmanStatus *status) 
{
    CMPIStatus rc;
    CMPIObjectPath * objectpath;    
    CMPIEnumeration * enumeration;
    CMCIClient *cc = cim_connect_to_cimom( "localhost", NULL, NULL , status);
    if (!cc) {
        return NULL;
    }

    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);

    enumeration = cc->ft->enumInstanceNames(cc, objectpath, &rc);
    debug( "enumInstanceNames() rc=%d, msg=%s",
            rc.rc, (rc.msg)? (char *)rc.msg->hdl : NULL);

    if (rc.rc) {
        debug( "CMCIClient enumInstanceNames() failed");
        cim_to_wsman_status(rc, status);
        return NULL;
    }
    CMPIArray * enumArr =  enumeration->ft->toArray(enumeration, NULL ); 
    debug( "Total enumeration items: %d", enumArr->ft->getSize(enumArr, NULL ));
    cim_to_wsman_status(rc, status);
    return enumArr;           
}


CMPICount
cim_enum_totalItems (CMPIArray * enumArr) 
{
    return enumArr->ft->getSize(enumArr, NULL );
}


char*
cim_get_property( CMPIInstance *instance,
                  char *property)
{
    CMPIStatus rc;
    CMPIData data = instance->ft->getProperty(instance, property, &rc);	
    char *valuestr = NULL;
    if (CMIsArray(data)) {
        return NULL;
    } else {
        if ( data.type != CMPI_null && data.state != CMPI_nullValue) {

            if (data.type !=  CMPI_ref) {
                valuestr = value2Chars(data.type, &data.value);
            }
        }
    }
    return valuestr;
}

char*
cim_get_keyvalue( CMPIObjectPath *objpath, 
                  char *keyname)
{
    CMPIStatus status;
    char *valuestr;
    debug( "Get key property from objpath");

    if (!objpath) {
        debug( "objpath is NULL");
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

void 
cim_get_enum_items(CimClientInfo *client,
                   WsContextH cntx,
                   WsXmlNodeH node,
                   WsEnumerateInfo* enumInfo,
                   char *namespace,
                   int max) 
{
    WsXmlNodeH itemsNode;
    if ( node != NULL ) {
        itemsNode = ws_xml_add_child(node, namespace, WSENUM_ITEMS, NULL);     	
        debug( "Total items: %d", enumInfo->totalItems );
        if (max > 0 ) {
            while(max > 0 && enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems) {
                if ( ( enumInfo->flags & FLAG_ENUMERATION_ENUM_EPR) == FLAG_ENUMERATION_ENUM_EPR )
                    cim_getEprAt(client, enumInfo, itemsNode);
                else if ( ( enumInfo->flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) == FLAG_ENUMERATION_ENUM_OBJ_AND_EPR )
                    cim_getEprObjAt(client, enumInfo, itemsNode);
                else
                    cim_getElementAt(client, enumInfo, itemsNode);
                enumInfo->index++;
                max--;
            }
            enumInfo->index--;
        } else {
            if ( enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems ) {
                if ( ( enumInfo->flags & FLAG_ENUMERATION_ENUM_EPR) == FLAG_ENUMERATION_ENUM_EPR )
                    cim_getEprAt(client, enumInfo, itemsNode);
                else if ( ( enumInfo->flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) == FLAG_ENUMERATION_ENUM_OBJ_AND_EPR )
                    cim_getEprObjAt(client, enumInfo, itemsNode);
                else
                    cim_getElementAt(client, enumInfo, itemsNode);
            }
        }
    }   
}


