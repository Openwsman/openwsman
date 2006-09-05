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

#include "wsman-util.h"

#include "wsman-errors.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"

#include "identify.h"

//
// ************ Serialization type information for resource ************
//
// It creates an array of items with name StateDescription_TypeInfo 
// It can be used in calls to WsSerialize and WsDeserialize 
//
SER_START_ITEMS("IdentifyResponse", wsmid_identify)
SER_STR("ProtocolVersion", 1, 1), 
SER_STR("ProductVendor", 1, 1),
SER_STR("ProductVersion", 1, 1),
SER_END_ITEMS("IdentifyResponse", wsmid_identify);


// ************** Array of end points for resource ****************
//
// Must follow general convention xxx_EndPoints
//

SER_START_END_POINTS(wsmid_identify)
END_POINT_IDENTIFY(wsmid_identify, XML_NS_WSMAN_ID ),
SER_FINISH_END_POINTS(wsmid_identify);



SER_START_NAMESPACES(wsmid_identify)
    ADD_NAMESPACE( XML_NS_WSMAN_ID ),
SER_FINISH_NAMESPACES(wsmid_identify);



void get_endpoints(GModule *self, void **data) 
{		 		
    WsDispatchInterfaceInfo *ifc = 	(WsDispatchInterfaceInfo *)data;	
    ifc->flags = 0;
    ifc->actionUriBase = NULL;
    ifc->version = PACKAGE_VERSION;
    ifc->vendor = "Openwsman Project";
    ifc->displayName = "IdentifyResponse";
    ifc->notes = "Return information about implementation";
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanResourceUri = NULL;
    ifc->extraData = NULL;
    ifc->namespaces = wsmid_identify_Namespaces;
    ifc->endPoints = wsmid_identify_EndPoints;			
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


