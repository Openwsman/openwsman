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

	const char *uuid;

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
		"Unsubscribe to a exsitent subscription",
		NULL,
		"/s:Envelope/s:Header/wsa:Action",
		"http://schemas.xmlsoap.org/ws/2004/08/eventing/UnsubscribeResponse",
		200,
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
	WsXmlDocH doc1, doc2;
	client_opt_t *options = NULL;

	if (getenv("OPENWSMAN_TEST_HOST")) {
		host = getenv("OPENWSMAN_TEST_HOST");
	}


	for (i = 0; i < ntests; i++)
	{
		printf ("Test %3d: %s ", i + 1, tests[i].explanation);
		cl= wsmc_create(
				sd[0].server,
				sd[0].port,
				sd[0].path,
				sd[0].scheme,
				sd[0].username,
				sd[0].password);
		wsmc_transport_init(cl, NULL);
		options = wsmc_options_init();
		options->delivery_uri = u_strdup("http://localhost:80/eventsink");
		options->delivery_mode = 1;
		//options->dialect = u_strdup("http://schemas.microsoft.com/wbem/wsman/1/WQL");
		//options->filter = u_strdup("select * from CIM_ProcessIndication");
		filter_t *filter = filter_create_simple("http://schemas.microsoft.com/wbem/wsman/1/WQL", "select * from CIM_ProcessIndication");
		doc1 = wsmc_action_subscribe(cl, "http://schemas.dmtf.org/wbem/wscim/1/*", options, filter);
		if(!doc1) {
			printf("\t\t\033[22;32msend request error!\033[m\n");
			goto CONTINUE;
		}
		const char *uuidstr = ws_xml_get_xpath_value(doc1, "/s:Envelope/s:Body/wse:SubscribeResponse/wse:SubscriptionManager/wsa:ReferenceParameters/wse:Identifier");
		doc2 = wsmc_action_unsubscribe(cl, "http://schemas.dmtf.org/wbem/wscim/1/*", options, uuidstr);
		if(!doc2) {
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
			char *xp = ws_xml_get_xpath_value(doc2, (char *)tests[i].xpath_expression);
			if (xp)
			{
				if (strncmp(xp,(char *)tests[i].expected_value, strlen((char *)tests[i].expected_value)) == 0)
					printf("\t\t\033[22;32mPASSED\033[m\n");
				else
					printf("%s = %s\t\033[22;31mFAILED\033[m\n",(char *)tests[i].xpath_expression, xp);
				u_free(xp);
			} else {
				ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc2));
				printf(" No %s\t\033[22;31mFAILED\033[m\n", (char *)tests[i].xpath_expression);

			}
		} else {
			printf("\t\t\033[22;32mPASSED\033[m\n");
		}

		ws_xml_destroy_doc(doc1);
		ws_xml_destroy_doc(doc2);
CONTINUE:
		wsmc_options_destroy(options);
		wsmc_release(cl);
	}
	return 0;
}


