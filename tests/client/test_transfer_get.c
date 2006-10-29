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



int facility = LOG_DAEMON;
int errors = 0;



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

		unsigned int auth_data;
		
} TestData;


ServerData sd[] = {
	{"localhost", 8889, "/wsman", "http", "wsman", "secret"}
};

TestData tests[] = {
	{
	    "Transfer Get without any selectors, Check Fault Value", 
	    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
	    NULL,
		"/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
		"wsman:InvalidSelectors",	    
	    500, 
	    0
	},
    {
        "Transfer Get without any selectors, Checking FaultDetail", 
        "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
        NULL,
	    "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
	    "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",	    
        500, 
        0
    },
    {
        "Transfer Get with non existent Resource URI, Check FaultDetail",
        "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx",
        NULL, 
        "/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
        "http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InvalidResourceURI",	    
        500, 
        0
    },    
	{
	    "Transfer Get with non existent Resource URI, Check Fault Value",
	    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx",
	    NULL, 
		"/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
		"wsa:DestinationUnreachable",  
	    500, 
	    0
	},	
	{
	    "Transfer Get with missing selectors, Checking Fault Value", 
	    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
	    "Name=langley.home.planux.com",
		"/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
		"wsman:InvalidSelectors",	    
	    500,
	    0
	},
    {
    "Transfer Get with missing selectors, Checking FaultDetail", 
    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
    "Name=langley.home.planux.com",
	"/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
	"http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/InsufficientSelectors",	    
    500,
    0
    },	
{
"Transfer Get with duplicate selectors, Checking FaultDetail", 
"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
"Name=langley.home.planux.com&Name=langley.home.planux.com1",
"/s:Envelope/s:Body/s:Fault/s:Detail/wsman:FaultDetail",
"http://schemas.dmtf.org/wbem/wsman/1/wsman/faultDetail/DuplicateSelectors",	    
500,
0
},	
	{   
	    "Transfer Get with all selectors but with wrong values Fault Value", 
	    "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
		"CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=langley.home.planux.comx",
		"/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
		"wsa:DestinationUnreachable",		
		500,
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

int ntests = sizeof (tests) / sizeof (tests[0]);

#if 0
static void wsman_output(WsXmlDocH doc)
{
	ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
	return;
}

#endif

int main(int argc, char** argv)
{
	int i;
	WsManClient *cl;
	WsXmlDocH doc;
	actionOptions options;
		
	
	wsman_client_transport_init(NULL);
	
		
	for (i = 0; i < ntests; i++) 
	{
		printf ("Test %d: %70s:", i + 1, tests[i].explanation);
    	WsContextH cntx = ws_create_runtime(NULL);

    	cl = wsman_connect( cntx, 
    		sd[0].server,
    		sd[0].port,
    		sd[0].path,
    		sd[0].scheme,
    		sd[0].username,
    		sd[0].password);		
		initialize_action_options(&options);
		
		if (tests[i].selectors != NULL)
			wsman_add_selectors_from_query_string (&options, tests[i].selectors);
		
		 
		doc = ws_transfer_get(cl, (char *)tests[i].resource_uri, options);
        if ((char *)tests[i].expected_value != NULL) 
        {			  
            char *xp = ws_xml_get_xpath_value(doc, (char *)tests[i].xpath_expression);
            if (xp)
            {
                if (strcmp(xp,(char *)tests[i].expected_value ) == 0)
                    printf("\t\t\033[22;32mPASSED\033[m\n");
                else
                    printf("\t\t\033[22;31mFAILED\033[m\n");	
                u_free(xp);		            
            } else {
                printf("\t\t\033[22;31mFAILED\033[m\n");
            }        
        }		
		if (doc) {			
			ws_xml_destroy_doc(doc);
		}
		
		destroy_action_options(&options);
        wsman_release_client(cl);
    	if (cntx) {
    		SoapH soap = ws_context_get_runtime(cntx);  
    		soap_destroy_fw(soap);    		
    	}		
	}
	
	return 0;
}


