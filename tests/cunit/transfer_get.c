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
    "Transfer Get without any selectors.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
    NULL,
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",	    
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",
    500, 
    0,
    0
  },

  {
    "Transfer Get with non existent Resource URI.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx",
    NULL, 
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",  
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",
    500, 
    0,
    0
  },

  {
    "Transfer Get with unsufficient selectors.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    "Name=%s",
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",
    500,
    0,
    0
  },

  {
    "Transfer Get with wrong selectors.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Namex=%s",
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/UnexpectedSelectors",
    500,
    0,
    0
  },

  {
    "Transfer Get with all selectors but with wrong values 1.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=%sx",
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidValue",
    500,
    0,
    0
  },
  {
    "Transfer Get with all selectors but with wrong values 2.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    "CreationClassName=OpenWBEM_UnitaryComputerSystemx&Name=%s",
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsman:InvalidSelectors",
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidValue",
    500,
    0,
    0
  },
  {
    "Transfer Get with correct selectors. Check response code",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    "CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=%s",
    NULL,
    NULL,
    NULL,
    NULL,
    200,
    0,
  },
};

static int ntests = sizeof (get_tests) / sizeof (get_tests[0]);



extern WsManClient *cl;
actionOptions options;

static void transfer_get_test() {
    WsXmlDocH doc;
    char *xpf = NULL;
    char *xpd = NULL;
    static int i = 0; // executed test number.
    char *old_selectors = get_tests[i].selectors;


    if (get_tests[i].selectors) {
        get_tests[i].selectors =
              u_strdup_printf(get_tests[i].selectors, host, host, host);
    }

    reinit_client_connection(cl);
    initialize_action_options(&options);

    if (get_tests[i].selectors != NULL) {
       wsman_add_selectors_from_query_string (&options, get_tests[i].selectors);
    }


    doc = ws_transfer_get(cl, (char *)get_tests[i].resource_uri, options);
    CU_ASSERT_TRUE(cl->response_code == get_tests[i].final_status);

    CU_ASSERT_PTR_NOT_NULL(doc);
    if (!doc) {
        goto RETURN;
    }

    if (get_tests[i].fault_expr == NULL) {
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(get_tests[i].fault_value);
    if (get_tests[i].fault_value == NULL) {
        goto RETURN;
    }
    xpf = ws_xml_get_xpath_value(doc, get_tests[i].fault_expr);
    CU_ASSERT_PTR_NOT_NULL(xpf);
    if (!xpf) {
        goto RETURN;
    }
    CU_ASSERT_STRING_EQUAL(xpf, get_tests[i].fault_value);

    if (strcmp(xpf, get_tests[i].fault_value)) {
        //printf("Expected %s;   returned %s\n",
        //           get_tests[i].fault_value, xpf);
         goto RETURN;
    }
    if (get_tests[i].detail_expr == NULL) {
        goto RETURN;
    }
    xpd = ws_xml_get_xpath_value(doc, get_tests[i].detail_expr);
    CU_ASSERT_PTR_NOT_NULL(xpd);
    if (!xpd) {
        goto RETURN;
    }
    CU_ASSERT_PTR_NOT_NULL(get_tests[i].detail_value);
    if (get_tests[i].detail_value == NULL) {
        goto RETURN;
    }
    CU_ASSERT_STRING_EQUAL(xpd, get_tests[i].detail_value );
    if (strcmp(xpd, get_tests[i].detail_value)) {
        //printf("Expected %s;   returned %s\n",
        //              get_tests[i].detail_value, xpd);
         goto RETURN;
    }

RETURN:
    u_free(xpf);
    u_free(xpd);
    if (doc) {
        ws_xml_destroy_doc(doc);
    }
    u_free((char *)get_tests[i].selectors);
    get_tests[i].selectors = old_selectors;
    destroy_action_options(&options);
    i++; // increase executed test number
}




int add_transfer_get_tests(CU_pSuite ps) {
    int found_test = 0;
    int i;
        /* add the tests to the suite */
    for (i =0; i < ntests; i++) {
            found_test += (NULL != CU_add_test(ps, get_tests[i].explanation,
                                            (CU_TestFunc)transfer_get_test));
    }
    return (found_test > 0);
}

