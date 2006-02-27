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


//
// ************ Serialization type information for resource ************
//
// It creates an array of items with name StateDescription_TypeInfo 
// It can be used in calls to WsSerialize and WsDeserialize 
//
SER_START_ITEMS("Intel_IPMI_sensor", IpmiSensor)
        SER_STR("SystemCreationClassName", 0, 1),
        SER_STR("SystemName", 0, 1),
        SER_STR("CreationClassName", 0, 1),
        SER_STR("DeviceID", 0, 1),

        SER_STR("ElementName", 0, 1),
        SER_STR("Name", 0, 1),
        SER_STR("SensorType", 0, 1),
        SER_STR("OtherSensorTypeDescription", 0, 1),
        SER_STR("CurrentState", 0, 1),
SER_END_ITEMS("Intel_IPMI_sensor", IpmiSensor);


// ************** Array of end points for resource ****************
//
// Must follow general convention xxx_EndPoints
//

START_TRANSFER_GET_SELECTORS(IpmiSensor)
        ADD_SELECTOR("SystemCreationClassName", "string", "System Creation Class Name"),
        ADD_SELECTOR("SystemName", "string", "System Name"),
	ADD_SELECTOR("CreationClassName", "string", "Creation Class Name"),
        ADD_SELECTOR("DeviceID", "string", "Device ID"),
FINISH_TRANSFER_GET_SELECTORS(IpmiSensor);

SER_START_END_POINTS(IpmiSensor)
	END_POINT_TRANSFER_GET(IpmiSensor, XML_NS_CIM_V2_9),
    END_POINT_TRANSFER_ENUMERATE(IpmiSensor, XML_NS_CIM_V2_9),
    END_POINT_TRANSFER_PULL(IpmiSensor, XML_NS_CIM_V2_9),
    END_POINT_TRANSFER_RELEASE(IpmiSensor, XML_NS_CIM_V2_9),
SER_FINISH_END_POINTS(IpmiSensor);


void get_endpoints(GModule *self, void **data)
{
        WsDispatchInterfaceInfo *ifc = (WsDispatchInterfaceInfo *)data;	
        ifc->flags = 0;
        ifc->actionUriBase = XML_NS_CIM_V2_9;
        ifc->version = PACKAGE_VERSION;
        ifc->vendor = "Intel Corp.";
        ifc->displayName = "Intel_IPMI_sensor";
        ifc->notes = "Intel_IPMI_sensor";
        ifc->compliance = XML_NS_WS_MAN;
        ifc->wsmanSystemUri = NULL;
        ifc->wsmanResourceUri = WS_IPMI_SENSOR_RESOURCE_URI;
        ifc->extraData = NULL;
        ifc->endPoints = IpmiSensor_EndPoints;	    	   
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







