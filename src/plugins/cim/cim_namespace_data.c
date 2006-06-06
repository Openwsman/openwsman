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

#include "cim_namespace_data.h"


SER_START_ITEMS("Catalog", CimResource)
SER_END_ITEMS("Catalog", CimResource);

SER_START_END_POINTS(CimResource)
    END_POINT_TRANSFER_GET_RAW(CimResource, XML_NS_CIM_CLASS),
    END_POINT_TRANSFER_ENUMERATE(CimResource, XML_NS_CIM_CLASS),
    END_POINT_TRANSFER_PULL_RAW(CimResource, XML_NS_CIM_CLASS),
    END_POINT_TRANSFER_RELEASE(CimResource, XML_NS_CIM_CLASS),
    END_POINT_CUSTOM_METHOD(CimResource, XML_NS_CIM_CLASS),
SER_FINISH_END_POINTS(CimResource);


void get_endpoints(GModule *self, void **data)
{
    WsDispatchInterfaceInfo *ifc = 	(WsDispatchInterfaceInfo *)data;	
    ifc->flags = 0;
    ifc->actionUriBase = XML_NS_CIM_CLASS;
    ifc->version = PACKAGE_VERSION;
    ifc->vendor = "Intel Corp.";
    ifc->displayName = "CIM Resource";
    ifc->notes = "CIM Resource";
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanSystemUri = NULL;
    ifc->wsmanResourceUri = NULL;
    ifc->extraData = NULL;
    ifc->endPoints = CimResource_EndPoints;	    	   
    return;
}

int init( GModule *self, void **data )
{
    return 1;
}

void cleanup( GModule *self, void *data )
{
	return;
}

