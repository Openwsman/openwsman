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
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-client.h"
#include "wsman-client-transport.h"



//int facility = LOG_DAEMON;
int errors = 0;



typedef struct {
	const char *server;
	int port;
	const char *path;
	const char *scheme;
	const char *username;
	const char *password;
} ServerData;




ServerData sd[] = {
	{"localhost", 8889, "/wsman", "http", "wsman", "secret"}
};



#if 0
static void wsman_output(WsXmlDocH doc)
{
	ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
	return;
}

#endif

int main(int argc, char** argv)
{
	
    WsManClient *cl;
    WsXmlDocH doc;
    client_opt_t *options = NULL;


    printf ("Test 1: Testin Identify Request:");
    cl = wsmc_create( sd[0].server,
        sd[0].port,
        sd[0].path,
        sd[0].scheme,
        sd[0].username,
        sd[0].password);		
    wsmc_transport_init(cl, NULL);
    options = wsmc_options_init();


    doc = wsmc_action_identify(cl, options);
    if (!doc) {
           printf("\t\t\033[22;31mUNRESOLVED\033[m\n");
           goto CONTINUE;
    }
    char *xp = ws_xml_get_xpath_value(doc, "/s:Envelope/s:Body/wsmid:IdentifyResponse/wsmid:ProtocolVersion");
    if (xp)
    {
        if (strcmp(xp,XML_NS_WS_MAN ) == 0)
            printf("\t\t\033[22;32mPASSED\033[m\n");
        else
            printf("\t\t\033[22;31mFAILED\033[m\n");	
        u_free(xp);		            
    } else {
        printf("\t\t\033[22;31mFAILED\033[m\n");
    }        

    if (doc) {			
        ws_xml_destroy_doc(doc);
    }
CONTINUE:
    wsmc_options_destroy(options);
    wsmc_release(cl);

	
	return 0;
}


