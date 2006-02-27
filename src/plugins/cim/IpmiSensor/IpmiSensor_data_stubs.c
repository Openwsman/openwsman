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
 * @author Haihao Xiang
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
#include "xml_serializer.h"
#include "ws_dispatcher.h"

#include "IpmiSensor_data.h"
#include "wsman-debug.h"
#include "sfcc-interface.h"
#include "interface_utils.h"

// ******************* Intel_IPMI_sensor *******************************
static IpmiSensor *set_values(CMPIInstance *instance)
{
	IpmiSensor *g_IpmiSensor = (IpmiSensor *)malloc(sizeof(IpmiSensor));
	
        g_IpmiSensor->SystemCreationClassName = cim_get_property(instance, "SystemCreationClassName");
        g_IpmiSensor->SystemName = cim_get_property(instance, "SystemName");
        g_IpmiSensor->CreationClassName = cim_get_property(instance, "CreationClassName");
        g_IpmiSensor->DeviceID = cim_get_property(instance, "DeviceID");

        g_IpmiSensor->ElementName = cim_get_property(instance, "ElementName");
        g_IpmiSensor->Name = cim_get_property(instance, "Name");
        g_IpmiSensor->SensorType = cim_get_property(instance, "SensorType");
        g_IpmiSensor->OtherSensorTypeDescription = cim_get_property(instance, "OtherSensorTypeDescription");
        g_IpmiSensor->CurrentState = cim_get_property(instance, "CurrentState");
	
	return g_IpmiSensor;
}



IpmiSensor* IpmiSensor_Get_EP(WsContextH cntx)
{
        IpmiSensor *g_IpmiSensor = (IpmiSensor *)malloc(sizeof(IpmiSensor));	
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Get Endpoint Called");
	
	// Keys
	GList *keys = create_key_list(cntx, IpmiSensor_Get_Selectors);
	
	if (keys)
	{
		wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, 
			"Number of keys defined: %d", g_list_length(keys));
	
		CimClientInfo cimclient;
		cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL );
		if (!cimclient.cc)
			return NULL;		
			
		char *CreationClassName = wsman_get_selector(cntx, NULL, "CreationClassName", 0);
		char *className;
		if (CreationClassName)			
			className = CreationClassName;
		else
			className = "Intel_IPMI_sensor";
		
		CMPIInstance *instance = cim_get_instance(cimclient.cc, className, keys);
			
		if (instance) 
		{					
			g_IpmiSensor = set_values(instance );	
			return g_IpmiSensor;			
		}    		
	} 
	return NULL;
	
}


int IpmiSensor_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Enumerate Endpoint Called"); 
	
	CimClientInfo cimclient;
	cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL );
	if (!cimclient.cc)
		return 1;
		
    CMPIArray * enumArr = cim_enum_instances (cimclient.cc, "Intel_IPMI_sensor" );
    if (!enumArr)
	    return 1;

    enumInfo->totalItems = cim_enum_totalItems(enumArr);
    enumInfo->enumResults = enumArr;
    return 0;
}





int IpmiSensor_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Release Endpoint Called");      
    return 0;
}

int IpmiSensor_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{ 
	IpmiSensor *g_IpmiSensor = (IpmiSensor *)malloc(sizeof(IpmiSensor));
	wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Pull Endpoint Called"); 
	
 	if ( enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems )
    {
    		CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
    		CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);
    		g_IpmiSensor = set_values(data.value.inst);
    		enumInfo->pullResultPtr = g_IpmiSensor;
    }
    else
    {    	
    		enumInfo->pullResultPtr = NULL;
    }

    return 0;
}



