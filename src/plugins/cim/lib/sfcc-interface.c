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

#include <glib.h>
#include <CimClientLib/cmci.h>
#include "ws_utilities.h"



#include "ws_errors.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_serializer.h"
#include "ws_dispatcher.h"

#include "sfcc-interface.h"
#include "wsman-debug.h"


extern char *value2Chars(CMPIType type, CMPIValue * value);


void cim_connect_to_cimom(
	CimClientInfo *cimclient,
	char *cim_host, 
	char *cim_host_userid, 
	char *cim_host_passwd )
{
	CMPIStatus status;    
   
    cimclient->cc = cmciConnect(cim_host, NULL, DEFAULT_HTTP_CIMOM_PORT,
                               cim_host_userid, cim_host_passwd, &status);
	if (cimclient->cc == NULL) 
	{
		wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Connection to CIMOM failed: %s", (char *)status.msg->hdl);		
	} 
}



CMPIInstance * cim_get_instance (CMCIClient *cc, 
					char *class_name, 
					GList *keys ) 
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


CMPIArray * cim_enum_instances (CMCIClient *cc, 
					char *class_name ) 
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
	return enumArr;           
    
}

CMPIArray *cim_enum_instancenames (CMCIClient *cc, 
					char *class_name ) 
{
	
    CMPIObjectPath * objectpath;    
    CMPIStatus status;
    CMPIEnumeration * enumeration;
    
    objectpath = newCMPIObjectPath(CIM_NAMESPACE, class_name, NULL);
       
    enumeration = cc->ft->enumInstanceNames(cc, objectpath, &status);
    

    /* Print the results */
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "enumInstanceNames() rc=%d, msg=%s",
            status.rc, (status.msg)? (char *)status.msg->hdl : NULL);
    
    
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "toArray() rc=%d, msg=%s",
            status.rc, (status.msg)? (char *)status.msg->hdl : NULL);
                  
    if (status.rc) {
	wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "CMCIClient enumInstanceNames() failed");
	return NULL;
    }
    CMPIArray * enumArr =  enumeration->ft->toArray(enumeration, &status ); 
    wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Total enumeration items: %lu", 
    		enumArr->ft->getSize(enumArr, NULL ));
	return enumArr;           
    
}


CMPICount cim_enum_totalItems (CMPIArray * enumArr) 
{
	return enumArr->ft->getSize(enumArr, NULL );
}


char* cim_get_property(CMPIInstance *instance, char *property)
{
 	CMPIStatus status;
 	char *valuestr;
 	wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Get Property from instance");

        if (!instance) 
        {
                wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "instance is NULL");
                return "";
        }

	CMPIData data = instance->ft->getProperty(instance, property, &status);	
	
	if (CMIsArray(data)) 
	{
		//free(valuestr);
		return "";
	}
	else 
	{
		valuestr = value2Chars(data.type, &data.value);
		return valuestr;
	}	
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
	
	if (status.rc || CMIsArray(data)) 
	{
		return "";
	}
	else 
	{
		valuestr = value2Chars(data.type, &data.value);
		return valuestr;
	}	
}

