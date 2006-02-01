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


//
// ************ Serialization type information for resource ************
//
// It creates an array of items with name StateDescription_TypeInfo 
// It can be used in calls to WsSerialize and WsDeserialize 
//
SER_START_ITEMS( "CIM_ComputerSystem", ComputerSystem)
	SER_STR("Caption", 0, 1),
	SER_STR("CreationClassName", 0, 1),
	SER_STR("Dedicated", 0, 1),
	SER_STR("Description", 0, 1),
	SER_STR("ElementName", 0, 1),
	SER_STR("EnabledDefault", 0, 1),
	SER_STR("EnabledState", 0, 1),
	SER_STR("HealthState", 0, 1),
	SER_STR("IdentifyingDescriptions", 0, 1),
	SER_STR("InstallDate", 0, 1),
	SER_STR("Name", 0, 1),
	SER_STR("NameFormat", 0, 1),
	SER_STR("OperationalStatus", 0, 1),
	SER_STR("OtherDedicatedDescriptions", 0, 1),
	SER_STR("OtherEnabledState", 0, 1),
	SER_STR("OtherIdentifyingInfo", 0, 1),
	SER_STR("PowerManagementCapabilities", 0, 1),
	SER_STR("PrimaryOwnerContact", 0, 1),
	SER_STR("PrimaryOwnerName", 0, 1),
	SER_STR("RequestedState", 0, 1),
	SER_STR("ResetCapability", 0, 1),
	SER_STR("Roles", 0, 1),
	SER_STR("Status", 0, 1),
	SER_STR("StatusDescriptions", 0, 1),
	SER_STR("TimeOfLastStateChange", 0, 1),
SER_END_ITEMS("CIM_ComputerSystem", ComputerSystem);


// ************** Array of end points for resource ****************
//
// Must follow general convention xxx_EndPoints
//

START_TRANSFER_GET_SELECTORS(ComputerSystem)
	ADD_SELECTOR("Name", "string", "Name"),
	ADD_SELECTOR("CreationClassName", "string", "Creation Class Name"),
FINISH_TRANSFER_GET_SELECTORS(ComputerSystem);

SER_START_END_POINTS(ComputerSystem)
	END_POINT_TRANSFER_GET(ComputerSystem, XML_NS_CIM_V2_9),
    END_POINT_TRANSFER_ENUMERATE(ComputerSystem, XML_NS_CIM_V2_9),
    END_POINT_TRANSFER_PULL(ComputerSystem, XML_NS_CIM_V2_9),
    END_POINT_TRANSFER_RELEASE(ComputerSystem, XML_NS_CIM_V2_9),
SER_FINISH_END_POINTS(ComputerSystem);


void get_endpoints(GModule *self, void **data)
{
	WsDispatchInterfaceInfo *ifc = 	(WsDispatchInterfaceInfo *)data;	
    ifc->flags = 0;
    ifc->actionUriBase = XML_NS_CIM_V2_9;
    ifc->version = PACKAGE_VERSION;
    ifc->vendor = "Intel Corp.";
    ifc->displayName = "CIM_ComputerSystem";
    ifc->notes = "CIM ComputerSystem";
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanSystemUri = NULL;
    ifc->wsmanResourceUri = WS_COMPUTER_SYSTEM_RESOURCE_URI;
    ifc->extraData = NULL;
    ifc->endPoints = ComputerSystem_EndPoints;	    	   
}


int init( GModule *self, void **data )
{
    return 1;
}

void
cleanup( GModule *self, void *data )
{
	return;
}







