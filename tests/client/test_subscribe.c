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
 * @author Liang Hou
 */
#include "wsman_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"
#include "wsman-client-api.h"
#include "wsman-client-transport.h"



//int facility = LOG_DAEMON;
int errors = 0;
char *host = "langley.home.planux.com";



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
    char *selectors;

    int deliverymode;

    char *deliveryNotifyTo;

    char *deliveryEndTo;

    char *dialect;

    char *filter;

    int expiration;

    int heartbeat;

    int sendbookmark;

    char *referenceXML;

    const char* xpath_expression;

    char* expected_value;

    /* What the final status code should be. */
    unsigned int final_status;

    unsigned int auth_data;

} TestData;


ServerData sd[] = {
    {"localhost", 8889, "/wsman", "http", "wsman", "secret"}
};

TestData tests[] = {
    {
        "Subscribe to the server (push mode)",
        "http://schema.openwsman.org/2006/openwsman/test",
        NULL,
	0,
	"http://localhost:80/eventsink",
	NULL,
	"http://schemas.microsoft.com/wbem/wsman/1/WQL",
	"select * from CIM_ProcessIndication",
	0,
	2,
	0,
	NULL,
        "/s:Envelope/s:Body/wse:SubscribeResponse/wse:SubscriptionManager/wsa:ReferenceParameters/wse:Identifier",
        "uuid:",
        200,
        0
    },
    {
    	"Subscribe to the server (pushwithack mode)",
        "http://schema.openwsman.org/2006/openwsman/test",
        NULL,
	1,
	"http://localhost:80/eventsink",
	NULL,
	"http://schemas.microsoft.com/wbem/wsman/1/WQL",
	"select * from CIM_ProcessIndication",
	3600,
	0,
	0,
	NULL,
        "/s:Envelope/s:Body/wse:SubscribeResponse/wse:SubscriptionManager/wsa:ReferenceParameters/wse:Identifier",
        "uuid:",
        200,
        0
    },
    {
    	"Subscribe to the server (events mode)",
        "http://schema.openwsman.org/2006/openwsman/test",
        NULL,
	2,
	"http://localhost:80/eventsink",
	NULL,
	"http://schemas.microsoft.com/wbem/wsman/1/WQL",
	"select * from CIM_ProcessIndication",
	3600,
	0,
	0,
	NULL,
        "/s:Envelope/s:Body/wse:SubscribeResponse/wse:SubscriptionManager/wsa:ReferenceParameters/wse:Identifier",
        "uuid:",
        200,
        0
    },
    {
    	"Subscribe to the server (pull mode)",
        "http://schema.openwsman.org/2006/openwsman/test",
        NULL,
	3,
	"http://localhost:80/eventsink",
	NULL,
	"http://schemas.microsoft.com/wbem/wsman/1/WQL",
	"select * from CIM_ProcessIndication",
	3600,
	0,
	0,
	NULL,
        "/s:Envelope/s:Body/wse:SubscribeResponse/wsen:EnumerationContext",
        "uuid:",
        200,
        0
    },
    {
    	"Subscribe to the server (with invalid NotifyTo URL)",
        "http://schema.openwsman.org/2006/openwsman/test",
        NULL,
	0,
	"localhost:80/eventsink",
	NULL,
	"http://schemas.microsoft.com/wbem/wsman/1/WQL",
	"select * from CIM_ProcessIndication",
	3600,
	0,
	0,
	NULL,
        "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
        "wsmb:DevliveryToUnusable",
        500,
        0
    },
    {
    "Subscribe to the server (with unsupported Dialect)",
        "http://schema.openwsman.org/2006/openwsman/test",
        NULL,
	0,
	"http://localhost:80/eventsink",
	NULL,
	"http://schemas.microsoft.com/wbem/wsman/1/WQL2",
	"select * from CIM_ProcessIndication",
	3600,
	0,
	0,
	NULL,
        "/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
        "wse:FilteringNotSupported",
        400,
        0
    }
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
    client_opt_t *options = NULL;
    filter_t *filter = NULL;

    if (getenv("OPENWSMAN_TEST_HOST")) {
        host = getenv("OPENWSMAN_TEST_HOST");
    }


    for (i = 0; i < ntests; i++)
    {
        if (tests[i].selectors) {
            tests[i].selectors = u_strdup_printf(tests[i].selectors, host, host, host);
        }
        if (tests[i].expected_value) {
            tests[i].expected_value = u_strdup_printf(tests[i].expected_value, host, host, host);
        }

        printf ("Test %3d: %s ", i + 1, tests[i].explanation);
        cl = wsmc_create(
                sd[0].server,
                sd[0].port,
                sd[0].path,
                sd[0].scheme,
                sd[0].username,
                sd[0].password);
        wsmc_transport_init(cl, NULL);
        options = wsmc_options_init();

       if (tests[i].selectors != NULL)
            wsmc_add_selectors_from_str (options, tests[i].selectors);
	if (tests[i].deliveryNotifyTo)
	options->delivery_uri = u_strdup(tests[i].deliveryNotifyTo);
	options->delivery_mode = tests[i].deliverymode;
	if(tests[i].sendbookmark)
		wsmc_set_action_option(options, FLAG_EVENT_SENDBOOKMARK);
       if(tests[i].heartbeat)
               options->heartbeat_interval = tests[i].heartbeat;
       if(tests[i].expiration)
	   	options->expires = tests[i].expiration;
       if(tests[i].referenceXML)
	   	options->reference = u_strdup(tests[i].referenceXML);
	if(tests[i].filter)
		filter = filter_create_simple(tests[i].dialect, tests[i].filter);
    doc = wsmc_action_subscribe(cl, tests[i].resource_uri, options, filter);
	if(!doc) {
		printf("\t\t\033[22;32msend request error!\033[m\n");
		goto CONTINUE;
	}
        if (tests[i].final_status != wsmc_get_response_code(cl)) {
            printf("Status = %ld \t\t\033[22;31mFAILED\033[m\n",
                                    wsmc_get_response_code(cl));
            goto CONTINUE;
        }
        if ((char *)tests[i].expected_value != NULL)
        {
		char *xp = ws_xml_get_xpath_value(doc, (char *)tests[i].xpath_expression);
            if (xp)
            {
                if (strncmp(xp,(char *)tests[i].expected_value, strlen((char *)tests[i].expected_value)) == 0)
                    printf("\t\t\033[22;32mPASSED\033[m\n");
                else
                    printf("%s = %s\t\033[22;31mFAILED\033[m\n",(char *)tests[i].xpath_expression, xp);
                u_free(xp);
            } else {
            	 ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
                printf(" No %s\t\033[22;31mFAILED\033[m\n", (char *)tests[i].xpath_expression);

            }
        } else {
           printf("\t\t\033[22;32mPASSED\033[m\n");
        }

        ws_xml_destroy_doc(doc);
CONTINUE:
	 u_free(tests[i].selectors);
        u_free(tests[i].expected_value);
        wsmc_options_destroy(options);
        wsmc_release(cl);
    }
    return 0;
}


