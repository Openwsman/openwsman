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
#include "xml_serializer.h"
#include "ws_dispatcher.h"

#include "ComputerSystem_data.h"
#include "wsman-debug.h"
#include "sfcc-interface.h"
#include "sfcc-interface_utils.h"

// ******************* CIM ComputerSystem *******************************
static ComputerSystem *set_values(CMPIInstance *instance)
{
    ComputerSystem *g_ComputerSystem = (ComputerSystem *)malloc(sizeof(ComputerSystem));

    g_ComputerSystem->Name =  cim_get_property(instance, "Name");
    g_ComputerSystem->Caption =  cim_get_property(instance, "Caption");	
    g_ComputerSystem->CreationClassName = cim_get_property(instance, "CreationClassName");
    g_ComputerSystem->Dedicated =  cim_get_property(instance, "Dedicated");
    g_ComputerSystem->PrimaryOwnerContact = cim_get_property(instance, "PrimaryOwnerContact");
    g_ComputerSystem->Description = cim_get_property(instance, "Description");
    g_ComputerSystem->PrimaryOwnerName = cim_get_property(instance, "PrimaryOwnerName");
    g_ComputerSystem->ElementName = cim_get_property(instance, "ElementName");
    g_ComputerSystem->EnabledDefault = cim_get_property(instance, "EnabledDefault");
    g_ComputerSystem->EnabledState = cim_get_property(instance, "EnabledState");
    g_ComputerSystem->HealthState = NULL;
    g_ComputerSystem->IdentifyingDescriptions = NULL;
    g_ComputerSystem->InstallDate = NULL;
    g_ComputerSystem->NameFormat = NULL;
    g_ComputerSystem->OperationalStatus = NULL;
    g_ComputerSystem->OtherDedicatedDescriptions = NULL;
    g_ComputerSystem->OtherEnabledState = NULL;
    g_ComputerSystem->OtherIdentifyingInfo = NULL;
    g_ComputerSystem->PowerManagementCapabilities = NULL;
    g_ComputerSystem->RequestedState = NULL;
    g_ComputerSystem->ResetCapability = NULL;
    g_ComputerSystem->Roles = NULL;
    g_ComputerSystem->Status = NULL;
    g_ComputerSystem->StatusDescriptions = NULL;
    g_ComputerSystem->TimeOfLastStateChange = NULL;

    return g_ComputerSystem;

}



ComputerSystem* ComputerSystem_Get_EP(WsContextH cntx)
{
    ComputerSystem *g_ComputerSystem = (ComputerSystem *)malloc(sizeof(ComputerSystem));	
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Get Endpoint Called");
    WsmanStatus *status = (WsmanStatus *)soap_alloc(sizeof(WsmanStatus *), 0 );

    // Keys
    GList *keys = wsman_get_selector_list(cntx, NULL);

    if (keys) {
        wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, 
                "Number of keys defined: %d", g_list_length(keys));

        CimClientInfo cimclient;
        cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL , status);
        if (!cimclient.cc)
            return NULL;		

        char *CreationClassName = wsman_get_selector(cntx, NULL, "CreationClassName", 0);
        char *className;
        if (CreationClassName)			
            className = CreationClassName;
        else
            className = "CIM_ComputerSystem";

        CMPIInstance *instance = cim_get_instance_raw(cimclient.cc, className, keys);

        if (instance) {					
            g_ComputerSystem = set_values(instance );	
            if (instance) CMRelease(instance);
            if (cimclient.cc) CMRelease(cimclient.cc);
            return g_ComputerSystem;			
        }    		
    } 
    return NULL;
}


int ComputerSystem_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    WsmanStatus *status = (WsmanStatus *)soap_alloc(sizeof(WsmanStatus *), 0 );
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Enumerate Endpoint Called"); 

    CimClientInfo cimclient;
    cim_connect_to_cimom(&cimclient, "localhost", NULL, NULL , status);
    if (!cimclient.cc)
        return 1;

    CMPIArray * enumArr = cim_enum_instances_raw (cimclient.cc, "CIM_ComputerSystem" );
    if (!enumArr) {
        if (cimclient.cc) CMRelease(cimclient.cc);
        return 1;
    }

    enumInfo->totalItems = cim_enum_totalItems(enumArr);
    enumInfo->enumResults = enumArr;
    if (cimclient.cc) CMRelease(cimclient.cc);
    return 0;
}





int ComputerSystem_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Release Endpoint Called");      
    return 0;
}

int ComputerSystem_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{ 
    ComputerSystem *g_ComputerSystem = (ComputerSystem *)malloc(sizeof(ComputerSystem));
    wsman_debug (WSMAN_DEBUG_LEVEL_DEBUG, "Pull Endpoint Called"); 

    if ( enumInfo->index >= 0 && enumInfo->index < enumInfo->totalItems ) {
        CMPIArray * results = (CMPIArray *)enumInfo->enumResults;
        CMPIData data = results->ft->getElementAt(results, enumInfo->index, NULL);
        g_ComputerSystem = set_values(data.value.inst);
        enumInfo->pullResultPtr = g_ComputerSystem;
    } else {    	
        enumInfo->pullResultPtr = NULL;
    }

    return 0;
}



