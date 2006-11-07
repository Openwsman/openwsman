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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-errors.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-client.h"
#include "wsman-client-transport.h"
#include <CUnit/Basic.h> 
#include "common.h"


TestData get_tests[] = {
  {
    "Transfer Get without any selectors, Check Fault Value", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",	    
    500, 
    0,
    0
  },
  {
    "Transfer Get without any selectors, Checking FaultDetail", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",	    
    500, 
    0,
    0
  },
  {
    "Transfer Get with non existent Resource URI, Check FaultDetail",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx",
    NULL, 
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",	    
    500, 
    0,
    0
  },    
  {
    "Transfer Get with non existent Resource URI, Check Fault Value",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx",
    NULL, 
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",  
    500, 
    0,
    0
  },	
  {
    "Transfer Get with missing selectors, Checking Fault Value", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    "Name=langley.home.planux.com",
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",	    
    500,
    0,
    0
  },
  {
    "Transfer Get with missing selectors, Checking FaultDetail", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    "Name=langley.home.planux.com",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",	    
    500,
    0,
    0
  },	
  {   
    "Transfer Get with all selectors but with wrong values Fault Value", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=langley.home.planux.comx",
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",		
    500,
    0,
    0
  }/*,	
     {
     "Transfer Get with correct selectors", 
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
     "CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=langley.home.planux.com",
     "/s:Envelope/s:Body/p:ComputerSystem",
     200,
     0
     }*/
};


WsManClient *cl;
actionOptions options;

static int transfer_get_test(int idx)
{
  int i = idx;
  WsXmlDocH doc;
  if (get_tests[i].selectors != NULL)
    wsman_add_selectors_from_query_string (&options, get_tests[i].selectors);


  doc = ws_transfer_get(cl, (char *)get_tests[i].resource_uri, options);
  if ((char *)get_tests[i].expected_value != NULL) 
  {			  
    if ((char *)get_tests[i].expected_value != NULL) 
    {                         
      char *xp = ws_xml_get_xpath_value(doc, (char *)get_tests[i].xpath_expression);
      CU_ASSERT_PTR_NOT_NULL(xp);
      if (xp)
      {
        CU_ASSERT_STRING_EQUAL(xp,(char *)get_tests[i].expected_value );
        u_free(xp);                     
      }            
    }		
    if (doc) {			
      ws_xml_destroy_doc(doc);
    }
  }		
  return 0;
}



static void transfer_get_test_0(void)
{
  transfer_get_test(0);
}
static void transfer_get_test_1(void)
{
  transfer_get_test(1);
}
static void transfer_get_test_2(void)
{
  transfer_get_test(2);
}
static void transfer_get_test_3(void)
{
  transfer_get_test(3);
}
static void transfer_get_test_4(void)
{
  transfer_get_test(4);
}
static void transfer_get_test_5(void)
{
  transfer_get_test(5);
}
static void transfer_get_test_6(void)
{
  transfer_get_test(6);
}

int add_transfer_get_tests(CU_pSuite ps)
{
  int found_test = 0;
  /* add the tests to the suite */
  found_test = (NULL != CU_add_test(ps, get_tests[0].explanation, (CU_TestFunc)transfer_get_test_0));
  found_test = (NULL != CU_add_test(ps, get_tests[1].explanation, (CU_TestFunc)transfer_get_test_1));
  found_test = (NULL != CU_add_test(ps, get_tests[2].explanation, (CU_TestFunc)transfer_get_test_2));
  found_test = (NULL != CU_add_test(ps, get_tests[3].explanation, (CU_TestFunc)transfer_get_test_3));
  found_test = (NULL != CU_add_test(ps, get_tests[4].explanation, (CU_TestFunc)transfer_get_test_4));
  found_test = (NULL != CU_add_test(ps, get_tests[5].explanation, (CU_TestFunc)transfer_get_test_5));
  found_test = (NULL != CU_add_test(ps, get_tests[6].explanation, (CU_TestFunc)transfer_get_test_6));
  
  return (found_test > 0);
}

