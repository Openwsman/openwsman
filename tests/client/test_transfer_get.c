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
		
        /* What the final status code should be. */
        unsigned int final_status;		

		unsigned int auth_data;
		
} TestData;


ServerData sd[] = {
	{"localhost", 8889, "/wsman", "http", "wsman", "secret"}
};

TestData tests[] = {
	{"Transfer Get without any selectors.", "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", NULL , 500, 0},
	
	{"Transfer Get with non existent Resource URI.", "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystemxx", NULL, 500, 0},
	
	{"Transfer Get with non-complete selectors.", "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", "Name=langley.home.planux.com",  500, 0},
	
	{"Transfer Get with all selectors/wrong values.", "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
		"CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=langley.home.planux.comx",  500, 0},
	
	{"Transfer Get with correct selectors.", "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem", 
		"CreationClassName=OpenWBEM_UnitaryComputerSystem&Name=langley.home.planux.com",  200, 0}
};

int ntests = sizeof (tests) / sizeof (tests[0]);


static void wsman_output(WsXmlDocH doc)
{
	ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
	return;
}


int main(int argc, char** argv)
{
	int i;
	WsManClient *cl;
	WsXmlDocH doc;
	actionOptions options;
		
	
	wsman_client_transport_init(NULL);
	WsContextH cntx = ws_create_runtime(NULL);

	cl = wsman_connect( cntx, 
		sd[0].server,
		sd[0].port,
		sd[0].path,
		sd[0].scheme,
		sd[0].username,
		sd[0].password,
		NULL);
	// wsman_client_add_handler(wsman_client_handler, NULL);
	
		
	for (i = 0; i < ntests; i++) 
	{
		printf ("Test %d: %s\n", i + 1, tests[i].explanation);
		
		initialize_action_options(&options);
		
		if (tests[i].selectors != NULL)
			wsman_add_selectors_from_query_string (&options, tests[i].selectors);	
		 
		doc = ws_transfer_get(cl, (char *)tests[i].resource_uri, options);
		
		if (doc) {
			wsman_output(doc);
			ws_xml_destroy_doc(doc);
		}
		
		destroy_action_options(&options);
	}
	wsman_release_client(cl);
	return 0;
}


