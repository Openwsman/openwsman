
/**
 * @author Sumeet Kukreja, Dell Inc.
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

	/* Namespace containing the class */
	const char *namespace;

	/* Resource UR to test against */
	const char *resource_uri;

	/* Selectors in the form of a URI query   key=value&key2=value2 */
	char *selectors;

	const char* xpath_expression;
	char* expected_value;

	/* What the final status code should be. */
	unsigned int final_status;		

	unsigned int auth_data;

} TestData;

TestData test = {
	"Test References.", 
	"root/mock",
	"http://mock.dell.com/wbem/wscim/1/cim-schema/2/Mock_ComputerSystem",
	"Name=MComp2&CreationClassName=Mock_ComputerSystem",
	"/s:Envelope/s:Body/s:Fault/s:Code/s:Subcode/s:Value",
	NULL,    
	500, 
	0
};

ServerData sd[] = {
	{"localhost", 8889, "/wsman", "http", "wsman", "secret"}
};

static void wsman_output(WsXmlDocH doc)
{
	ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc));
	return;
}

int main(int argc, char** argv)
{
	WsManClient *cl;
	WsXmlDocH ref_resp = NULL, doc = NULL;
	actionOptions *options;
	char *enumContext = NULL;


	cl = wsman_create_client(sd[0].server, sd[0].port, sd[0].path, sd[0].scheme, sd[0].username, sd[0].password);		
	wsman_client_transport_init(cl, NULL);

	options = initialize_action_options();
	options->cim_ns = u_strdup(test.namespace);
	wsman_add_selectors_from_query_string (options, test.selectors);
	ref_resp = wsenum_enumerate(cl, (char *)test.resource_uri, options); 
	wsman_output(ref_resp);


	/* Pull for the response */
	enumContext = wsenum_get_enum_context(ref_resp);
	ws_xml_destroy_doc(ref_resp);

	while(enumContext != NULL) { 			
		doc = wsenum_pull(cl, (char *)test.resource_uri, options, enumContext);
		if (!doc) {
			printf("\t\t\033[22;31mUNRESOLVED\033[m\n");
			destroy_action_options(options);
			wsman_release_client(cl);
			return -1;
		}
		wsman_output(doc);	
		enumContext = wsenum_get_enum_context(doc);
	}
	if (doc)
		ws_xml_destroy_doc(doc);
	destroy_action_options(options);
	wsman_release_client(cl);
	return 0;
}


