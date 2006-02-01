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

#include "wsman_catalog.h"

//
// ************ Serialization type information for resource ************
//
// It creates an array of items with name StateDescription_TypeInfo 
// It can be used in calls to WsSerialize and WsDeserialize 
//
SER_START_ITEMS("Catalog", WsManCatalog)
SER_END_ITEMS("Catalog", WsManCatalog);


START_TRANSFER_GET_SELECTORS(WsManCatalog)
	ADD_SELECTOR("ResourceUri", "string", "Resource URI"),
FINISH_TRANSFER_GET_SELECTORS(WsManCatalog);

// ************** Array of end points for resource ****************
//
// Must follow general convention xxx_EndPoints
//
SER_START_END_POINTS(WsManCatalog)
	END_POINT_TRANSFER_GET_RAW(WsManCatalog, XML_NS_WS_MAN_CAT),
    END_POINT_TRANSFER_ENUMERATE(WsManCatalog, XML_NS_WS_MAN_CAT),
    END_POINT_TRANSFER_PULL_RAW(WsManCatalog, XML_NS_WS_MAN_CAT),
    END_POINT_TRANSFER_RELEASE(WsManCatalog, XML_NS_WS_MAN_CAT),           
SER_FINISH_END_POINTS(WsManCatalog);





void get_endpoints(GModule *self, void **data) 
{		
	WsDispatchInterfaceInfo *ifc = 	(WsDispatchInterfaceInfo *)data;	
    ifc->flags = 0;
    ifc->actionUriBase = XML_NS_WS_MAN;
    ifc->version = PACKAGE_VERSION;
    ifc->vendor = "Intel Corp.";
    ifc->displayName = "Catalog";
    ifc->notes = "WS-Management Catalog 2005/06";
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanSystemUri = NULL;
    ifc->wsmanResourceUri = WS_MAN_CATALOG_RESOURCE_URI;
    ifc->extraData = NULL;
    ifc->endPoints = WsManCatalog_EndPoints;	
		
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

