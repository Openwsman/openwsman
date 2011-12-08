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
#include "wsman_config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"


#include "u/libu.h"

#include "wsman-errors.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"

#include "wsman_test2.h"

//
// ************ Serialization type information for resource ************
//
// It creates an array of items with name StateDescription_TypeInfo 
// It can be used in calls to WsSerialize and WsDeserialize 
//


SER_START_ITEMS("Test2", WsManTest2)
SER_STR("Testing", 1, 1), 
SER_END_ITEMS("Test2", WsManTest2);


// ************** Array of end points for resource ****************
//
// Must follow general convention xxx_EndPoints
//

START_TRANSFER_GET_SELECTORS(WsManTest2)
FINISH_TRANSFER_GET_SELECTORS(WsManTest2);

SER_START_END_POINTS(WsManTest2)
    END_POINT_TRANSFER_GET(WsManTest2, XML_NS_WS_MAN"/test2"),
    END_POINT_TRANSFER_ENUMERATE(WsManTest2, XML_NS_WS_MAN"/test2"),
    END_POINT_TRANSFER_PULL(WsManTest2, XML_NS_WS_MAN"/test2"),
    END_POINT_TRANSFER_RELEASE(WsManTest2, XML_NS_WS_MAN"/test2"),
    END_POINT_TRANSFER_PUT(WsManTest2, XML_NS_WS_MAN"/test2"),
SER_FINISH_END_POINTS(WsManTest2);





void get_endpoints(GModule *self, void **data) 
{		
    WsDispatchInterfaceInfo *ifc = 	(WsDispatchInterfaceInfo *)data;	
    ifc->flags = 0;
    ifc->actionUriBase = XML_NS_WS_MAN;
    ifc->version = OPENWSMAN_PLUGIN_API_VERSION;
    ifc->vendor = "Intel Corp.";
    ifc->displayName = "Test2";
    ifc->notes = "Test2 Plugin";
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanSystemUri = NULL;
    ifc->wsmanResourceUri = WS_MAN_TEST_RESOURCE_URI;
    ifc->extraData = NULL;
    ifc->endPoints = WsManTest2_EndPoints;
}


int init( GModule *self, void **data )
{
    return 1;
}

void
cleanup( GModule *self, void *data )
{
}

