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






TestData identify_tests[] = {
  {
    "Testing Identify Request, check protocol version", 
    NULL, 
    NULL, 
    "/s:Envelope/s:Body/wsmid:IdentifyResponse/wsmid:ProtocolVersion",
    XML_NS_WS_MAN,
    200,
    FLAG_NONE,
    0
  },
  {
    "Testing Identify Request, check product version", 
    NULL, 
    NULL, 
    "/s:Envelope/s:Body/wsmid:IdentifyResponse/wsmid:ProductVersion",
    PACKAGE_VERSION,
    200,
    FLAG_NONE,
    0
  },
  {
    "Testing Identify Request, check product vendor", 
    NULL, 
    NULL, 
    "/s:Envelope/s:Body/wsmid:IdentifyResponse/wsmid:ProductVendor",
    "Openwsman Project",
    200,
    FLAG_NONE,
    0
  }
};

WsManClient *cl;
actionOptions options;



static int 
identify_test(int idx)
{
	
   
    WsXmlDocH response;    
    int i = idx;
   
    response = wsman_identify(cl, options);
    CU_ASSERT_TRUE(cl->response_code == identify_tests[i].final_status );

    if ((char *)identify_tests[i].expected_value != NULL) 
    {			  
      char *xp = ws_xml_get_xpath_value(response, (char *)identify_tests[i].xpath_expression);
      CU_ASSERT_PTR_NOT_NULL(xp);
      if (xp)
      {
        CU_ASSERT_STRING_EQUAL(xp,(char *)identify_tests[i].expected_value );
        u_free(xp);		            
      }            
    }		
    if (response) {			
      ws_xml_destroy_doc(response);
    }

	
	return 0;
}



static void identify_test_0(void)
{
  identify_test(0);
}
static void identify_test_1(void)
{
  identify_test(1);
}
static void identify_test_2(void)
{
  identify_test(2);
}

int add_identify_tests(CU_pSuite ps)
{
  int found_test = 0;
  /* add the tests to the suite */
  found_test = (NULL != CU_add_test(ps, identify_tests[0].explanation, (CU_TestFunc)identify_test_0));
  found_test = (NULL != CU_add_test(ps, identify_tests[1].explanation, (CU_TestFunc)identify_test_1));
  found_test = (NULL != CU_add_test(ps, identify_tests[2].explanation, (CU_TestFunc)identify_test_2));
  
  return (found_test > 0);
}


