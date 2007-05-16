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

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"

#include "wsman_test.h"



WsManTest g_WsManTest1 = { "Simple Test" , { 0, 0 } };
WsManTest g_WsManTest2 = { "Simple Test 1" , {0, 0}};
WsManTest g_WsManTest3 = { "Simple Test 2" , { 0, 0} };

WsManTest* g_WsManTestArr[2] = 
 {
 	&g_WsManTest2, &g_WsManTest3
 };

WsManTest* WsManTest_Get_EP(WsContextH cntx)
{
    debug ("Test Get Endpoint Called"); 
    return &g_WsManTest1;
}

WsManTest* WsManTest_Put_EP(WsContextH cntx)
{
    debug( "Put Endpoint Called"); 
    return &g_WsManTest1;
}

int WsManTest_Enumerate_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    debug( "Enumerate Endpoint Called");
    enumInfo->totalItems = 2;   
    return 0;
}

int WsManTest_Release_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{
    debug( "Release Endpoint Called");    
    return 0;
}

int WsManTest_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo)
{ 
    debug( "Pull Endpoint Called"); 
    if ( enumInfo->index >= 0 && enumInfo->index < 2 ){
        enumInfo->pullResultPtr = g_WsManTestArr[enumInfo->index];
    } else {    	
        enumInfo->pullResultPtr = NULL;
    }

    return 0;
}




