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
 * @author Nathan Rakoff
 */
#include "wsman_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <CUnit/Basic.h>

#include "u/libu.h"

#include "wsman-xml-api.h"

#include "wsman-soap-api.h"
#include "wsman-client-api.h"

#include "wsman-client-transport.h"
#include "wsman-debug.h"
#include "wsman-xml.h"
#include "enumeration.h"


int _debug = 0;

#define INVALID_RURI "wsa:DestinationUnreachablex"
#define XPATH_V "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value"

int facility = LOG_DAEMON;
int errors = 0;
unsigned char optimized_flags;

typedef struct {
  const char *server;
  int port;
  const char *path;
  const char *scheme;
  const char *username;
  const char *password;
} ServerData;

typedef struct {						
  /* Explanation of what you should see */
  const char *explanation;

  /* Resource UR to test against */
  const char *resource_uri;

  /* Selectors in the form of a URI query   key=value&key2=value2 */
  const char *selectors;

  const char* xpath_expression;
  const char* expected_value;

  /* What the final status code should be. */
  unsigned int final_status;		

  unsigned char       flags;

  unsigned int		max_elements;

} TestData;


ServerData sd[] = {
  {"localhost", 8889, "/wsman", "http", "wsman", "secret"}
};

TestData tests[] = {
  {
    "Enumeration with non existent Resource URI, Checking Fault Subcode", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx", 
    NULL, 
    "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
    "wsa:DestinationUnreachable",
    500,
    FLAG_NONE,
    0
  },
  {
    "Enumeration with non existent Resource URI, Checking FaultDetail", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx", 
    NULL, 
    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",
    500,
    FLAG_NONE,
    0
  },	
  {
    "Enumeration with valid Resource URI and Items Count Estimation.",
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    NULL, 
    "/s:Envelope/s:Header/wsman:TotalItemsCountEstimate",
    "2",
    200,
    FLAG_ENUMERATION_COUNT_ESTIMATION,
    0
  }/*,    
     {
     "Enumeration with valid Resource URI.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_NONE,
     0
     },
     {
     "Enumeration with valid Resource URI and additional invalid selectors.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_NONE,
     1
     },	
     {
     "Enumeration with valid Resource URI/Count Estimation.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_COUNT_ESTIMATION,
     0
     },
     {
     "Enumeration with valid Resource URI/Optimization.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_OPTIMIZATION,
     0
     },	
     {
     "Enumeration with Count Estimation/Optimzation and get all elements.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_COUNT_ESTIMATION,
     10

     },
     {
     "Enumeration with Count Estimation/Optimzation/Epr and get all elements.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_COUNT_ESTIMATION | FLAG_ENUMERATION_ENUM_EPR,
     10

     },	
     {
     "Enumeration with Count Estimation/Optimzation/ObjAndEpr and get all elements.",
     "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
     NULL, 
     200,
     FLAG_ENUMERATION_OPTIMIZATION | FLAG_ENUMERATION_COUNT_ESTIMATION | FLAG_ENUMERATION_ENUM_OBJ_AND_EPR,
     10

     }	*/
};


int ntests = sizeof (tests) / sizeof (tests[0]);

static void
debug_message_handler (const char *str, debug_level_e level, void  *user_data)
{
  if (wsman_debug_level_debugged(level)) 
    {
      struct tm *tm;
      time_t now;
      char timestr[128];

      time (&now);
      tm = localtime (&now);
      strftime (timestr, 128, "%b %e %T", tm);
      fprintf (stderr, "%s  %s\n", timestr, str);
    }
}

static void
initialize_logging (void)
{
  debug_add_handler (debug_message_handler, DEBUG_LEVEL_ALWAYS, NULL);
} 

static void wsman_output(WsXmlDocH doc)
{
  return;
  if (doc)
    ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
  else
    printf("returned doc is null\n");
  return;
}

WsManClient *cl;
actionOptions options;

int init_enumeration_test(void)
{
  wsman_client_transport_init(NULL);
  if (_debug) wsman_debug_set_level(DEBUG_LEVEL_DEBUG);
  initialize_logging();

  cl = wsman_create_client( 
		      sd[0].server,
		      sd[0].port,
		      sd[0].path,
		      sd[0].scheme,
		      sd[0].username,
		      sd[0].password);

  initialize_action_options(&options);
  return 0;
}

void enumeration_test(int idx);

void enumeration_test(int idx)
{
  char *enumContext = NULL;

  int i = idx;

  options.flags = tests[i].flags;

  if (tests[i].selectors != NULL)
    wsman_add_selectors_from_query_string (&options, tests[i].selectors);	

  options.max_elements = tests[i].max_elements;
  WsXmlDocH enum_response = wsenum_enumerate(cl, (char *)tests[i].resource_uri ,
						      options);
  if (enum_response) 
    {
      enumContext = wsenum_get_enum_context(enum_response);
    } else {
    enumContext = NULL;
  }
  if (_debug) wsman_output(enum_response);

  if ((char *)tests[i].expected_value != NULL) 
    {			  
      char *xp = ws_xml_get_xpath_value(enum_response, (char *)tests[i].xpath_expression);
      CU_ASSERT_PTR_NOT_NULL(xp);
      if (xp)
        {
	  CU_ASSERT_STRING_EQUAL(xp,(char *)tests[i].expected_value );
	  u_free(xp);		            
        }            
    }		
  ws_xml_destroy_doc(enum_response);
}

int clean_enumeration_test(void)
{			
  destroy_action_options(&options);		
  wsman_release_client(cl);
  return 0;
}

void enum_test_0(void);
void enum_test_1(void);
void enum_test_2(void);

void enum_test_0(void)
{
  enumeration_test(0);
}

void enum_test_1(void)
{
  enumeration_test(1);
}

void enum_test_2(void)
{
  enumeration_test(2);
}

int add_enumeration_tests(CU_pSuite ps)
{
  int found_test = 0;
  /* add the tests to the suite */
  found_test = (NULL != CU_add_test(ps, tests[0].explanation, (CU_TestFunc)enum_test_0));
  found_test += (NULL != CU_add_test(ps, tests[1].explanation, (CU_TestFunc)enum_test_1));
  found_test += (NULL != CU_add_test(ps, tests[2].explanation, (CU_TestFunc)enum_test_2));
  return (found_test > 0);
}


