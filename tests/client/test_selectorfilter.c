
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
	"Test SelectorSet filter.",
	NULL,
	"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem",
	"Name=vamt-build.sh.intel.com&CreationClassName=Linux_ComputerSystem",
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
	WsXmlDocH selector_resp = NULL, doc = NULL;
	client_opt_t *options;
	char *enumContext = NULL;

	cl = wsmc_create(sd[0].server, sd[0].port, sd[0].path, sd[0].scheme, sd[0].username, sd[0].password);
	wsmc_transport_init(cl, NULL);

	options = wsmc_options_init();
	if(test.namespace)
		options->cim_ns = u_strdup(test.namespace);
	hash_t *selectors = u_parse_query(test.selectors);
	hash_t *selectors_new = hash_create2(HASHCOUNT_T_MAX, 0, 0);
	hnode_t        *hn;
	hscan_t         hs;
	selector_entry *entry;
	hash_scan_begin(&hs, selectors);
	while ((hn = hash_scan_next(&hs))) {
		entry = u_malloc(sizeof(selector_entry));
		entry->type = 0;
		entry->entry.text = (char *)hnode_get(hn);
		hash_alloc_insert(selectors_new, hnode_getkey(hn), entry);
	}
	filter_t *filter = filter_create_selector(selectors_new);
	wsmc_set_action_option(options, FLAG_DUMP_REQUEST);
	selector_resp = wsmc_action_enumerate(cl, (char *)test.resource_uri, options, filter);

	if(!selector_resp) {
		printf("\t\t\033[22;31mUNRESOLVED\033[m\n");
		exit(0);
	}
	wsman_output(selector_resp);


	/* Pull for the response */
	enumContext = wsmc_get_enum_context(selector_resp);
	ws_xml_destroy_doc(selector_resp);

	while(enumContext != NULL) {
		doc = wsmc_action_pull(cl, (char *)test.resource_uri, options, NULL, enumContext);
		if (!doc) {
			printf("\t\t\033[22;31mUNRESOLVED\033[m\n");
			wsmc_options_destroy(options);
			wsmc_release(cl);
			return -1;
		}
		wsman_output(doc);
		enumContext = wsmc_get_enum_context(doc);
	}
	if (doc)
		ws_xml_destroy_doc(doc);
	wsmc_options_destroy(options);
	wsmc_release(cl);
	return 0;
}


