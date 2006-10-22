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

#include <wsman_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "u/libu.h"


#include "wsman-xml-api.h"
#include "wsman-errors.h"
#include "wsman-soap.h"
#include "wsman-client-options.h"
#include "wsman.h"

#if 0
static char *auth_methods[] = {
     "basic",
     "digest",
     "ntlm",
     NULL,
};
#endif

static const char **wsman_argv = NULL;

static int server_port =  80;
static char *cafile = NULL;
static int debug_level = -1;
static char *test_case = NULL;
static int enum_max_elements = 0;
char  enum_optimize = 0;
char  enum_estimate = 0;
char  dump_request = 0;
static char *enum_mode = NULL;
static char *binding_enum_mode = NULL;
//gboolean  dump_response = FALSE;

static char *username = NULL;
static char *password = NULL;
static char *server = "localhost";
static char *cim_namespace = NULL;
static char *fragment = NULL;
static char *wsm_filter = NULL;
static char *wsm_dialect = NULL;

static unsigned long operation_timeout = 0;
static unsigned long max_envelope_size = 0;

static char *_action = NULL;
static char *agent = NULL;
static char *config_file = NULL;
static char *output_file = NULL;
static char *resource_uri = NULL;
static char *invoke_method = NULL;
static char *url_path = NULL;
static char **properties = NULL;
static char *authentication_method = NULL;
static char no_verify_peer = 0;
static char *proxy = NULL;
static char *proxy_upwd = NULL;

WsActions action_data[] = 
{ 
 { "get", ACTION_TRANSFER_GET},
 { "put", ACTION_TRANSFER_PUT},
 { "enumerate", ACTION_ENUMERATION},
 { "invoke", ACTION_INVOKE},
 { "identify", ACTION_IDENTIFY},
 { "test", ACTION_TEST},
 { NULL, 0},
};

char wsman_parse_options(int argc, char **argv) 
{

    char retval = 0;
    u_error_t *error = NULL;

    u_option_entry_t options[] = {
	{ "debug",	'd',	U_OPTION_ARG_INT,	&debug_level,
		"Set the verbosity of debugging output.",	"1-6" },
	{ "cafile",	'c',	U_OPTION_ARG_STRING,	&cafile,
		"Certificate file",	"<filename>"  },
	{ "username",	'u',	U_OPTION_ARG_STRING,	&username,
		"User name",	"<username>" },
	{ "path",	'g',	U_OPTION_ARG_STRING,	&url_path,
		"Path",	"<path>" },
	{ "password",	'p',	U_OPTION_ARG_STRING,	&password,
		"Password",	"<password>" },
	{ "hostname",	'h',	U_OPTION_ARG_STRING,	&server,
		"Host name",	"<hostname>" },
	{ "port",	'P',	U_OPTION_ARG_INT,	&server_port,
		"Server Port",	"<port>"},
	{ "proxy",	'X',	U_OPTION_ARG_STRING,	&proxy,
		"Proxy name",			"<proxy>" },
	{ "proxyauth",	'Y',	U_OPTION_ARG_STRING,	&proxy_upwd,
		"Proxy user:pwd",		"<proxyauth>" },
	{ "auth",	'y', 	U_OPTION_ARG_STRING,	&authentication_method,
		"Authentication Method",	"<basic|digest>" },
	{ "method",	'a',	U_OPTION_ARG_STRING,	&invoke_method,
		"Method (Works only with 'invoke')", "<custom method>" },
	{ "prop",	'k',	U_OPTION_ARG_STRING_ARRAY,	&properties,
	"Properties with key value pairs (For 'put', 'invoke' and 'create')",
						"<key=val>" },
	{ "config-file",'C',	U_OPTION_ARG_STRING,	&config_file,
		"Alternate configuration file",	"<file>" },
	{ "out-file",	'O',	U_OPTION_ARG_STRING,	&output_file,
		"Write output to file",		"<file>" },
	{ "noverifypeer",'V',	U_OPTION_ARG_NONE,	&no_verify_peer,
		"Not to verify peer certificate",	NULL },
        { NULL }
    };



    u_option_entry_t request_options[] = {
	{ "filter",	'x',	U_OPTION_ARG_STRING,	&wsm_filter,
		"Filter" ,			"<filter>" },
	{ "dialect",	'D',	U_OPTION_ARG_STRING,	&wsm_dialect,
		"Filter Dialect" ,		"<dialect>" },
	{ "timeout",	't',	U_OPTION_ARG_INT,	&operation_timeout,
		"Operation timeout in seconds",	"<time in sec>" },
	{ "max-envelope-size", 'e', U_OPTION_ARG_INT,	&max_envelope_size,
		"maximal envelope size" ,	"<size>" },
	{ "fragment",	'F',	U_OPTION_ARG_STRING,	&fragment,
		"Fragment (Supported Dialects: XPATH)" , "<fragment>" },
        { NULL }
    };

    u_option_entry_t enum_options[] = {
#ifdef DMTF_WSMAN_SPEC_1
	{ "max-elements", 'm',	U_OPTION_ARG_INT,	&enum_max_elements,
	"Max Elements Per Pull/Optimized Enumeration",
	"<max number of elements>" },
	{ "optimize",	'o',	U_OPTION_ARG_NONE,	&enum_optimize,
		"Optimize enumeration results",		NULL },
	{ "estimate-count", 'E', U_OPTION_ARG_NONE,	&enum_estimate,
		"Return estimation of total items",	NULL },
	{ "enum-mode",	'M',	U_OPTION_ARG_STRING,	&enum_mode,
		"Enumeration Mode",	"epr|objepr" },
#endif
        { NULL }
    };

    u_option_entry_t cim_options[] = {
#ifdef DMTF_WSMAN_SPEC_1
	{ "namespace",	'N',	U_OPTION_ARG_STRING,	&cim_namespace,
	"CIM Namespace (default is root/cimv2)",	"<namespace>" },
	{ "binding-enum-mode", 'B', U_OPTION_ARG_STRING, &binding_enum_mode,
	"CIM binding Enumeration Mode",	"none|include|exclude"},
#endif
        { NULL }
    };

    u_option_entry_t test_options[] = {
	{ "from-file",	'f',	U_OPTION_ARG_STRING,	&test_case,
	"Send request from file",	"<file name>"},
	{ "print-request", 'R', U_OPTION_ARG_NONE,	&dump_request,
	"print request on stdout",	NULL},
        //{ "print-response", 'N', 0, G_OPTION_ARG_NONE, &dump_response, "print all responses to stdout", NULL},
        { NULL }
    };

    u_option_group_t *enum_group;
    u_option_group_t *test_group;
    u_option_group_t *cim_group;
    u_option_group_t *req_flag_group;

    u_option_context_t *opt_ctx;	
    opt_ctx = u_option_context_new("<action> <Resource Uri>");
    enum_group = u_option_group_new("enumeration", "Enumeration",
                                    "Enumeration Options");
    test_group = u_option_group_new("tests", "Tests", "Test Cases");
    cim_group = u_option_group_new("cim", "CIM", "CIM Options");
    req_flag_group = u_option_group_new("flags", "Flags", "Request Flags");

    u_option_group_add_entries(enum_group, enum_options);
    u_option_group_add_entries(test_group, test_options);
    u_option_group_add_entries(cim_group, cim_options);
    u_option_group_add_entries(req_flag_group, request_options);

    u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    u_option_context_add_main_entries(opt_ctx, options, "wsman");
    u_option_context_add_group(opt_ctx, enum_group);
    u_option_context_add_group(opt_ctx, test_group);
    u_option_context_add_group(opt_ctx, cim_group);
    u_option_context_add_group(opt_ctx, req_flag_group);

    retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);
    u_option_context_free(opt_ctx);

    if (error) {
        if (error->message)
            printf ("%s\n", error->message);
        u_error_free(error);
        return FALSE;
    }

    if (argc > 2 ) {
        _action = argv[1];
        resource_uri = argv[2];
    } else {
        if (argv[1] && ( strcmp(argv[1], "identify") == 0 || strcmp(argv[1], "test") == 0 )) {
            _action = argv[1];
            if (wsman_options_get_test_file() &&  strcmp(argv[1], "test") == 0 ) {
                printf("running test case from file\n");
            }
        } else {
            fprintf(stderr, "Error: operation can not be completed. Action or/and Resource Uri missing.\n");
            return FALSE;
        }
    }
    u_error_free(error);
    return TRUE;
}

const char * wsman_options_get_output_file (void)
{
    return output_file;
}
const char * wsman_options_get_config_file (void) {
    return config_file;
}

int wsman_read_client_config (dictionary *ini)
{
    if (iniparser_find_entry(ini, "client")) {
        agent = iniparser_getstr(ini, "client:agent");
        server_port = server_port ? server_port : iniparser_getint(ini, "client:port", 80);
        authentication_method = authentication_method? authentication_method: iniparser_getstr(ini, "client:authentication_method");
    } else {
        return 0;
    }
    return 1;
}

const char ** wsman_options_get_argv (void) {
    return wsman_argv;
}

int wsman_options_get_debug_level (void) {
    return debug_level;
}

int wsman_options_get_server_port (void) {
    return server_port;
}

unsigned long wsman_options_get_max_envelope_size (void) {
    return max_envelope_size;
}
unsigned long wsman_options_get_operation_timeout (void) {
    return operation_timeout;
}

char* wsman_options_get_cafile (void) {
    return cafile;
}        

char* wsman_options_get_server (void) {
    if (server)
        return server;
    else
        return "localhost";
}

char* wsman_options_get_invoke_method (void) {
    return invoke_method;
}   

char* wsman_options_get_username (void) {
    return username;
}   

char* wsman_options_get_password (void) {
    return password;
}
 
char*  wsman_options_get_proxy(void) {
    return proxy;
}

char*  wsman_options_get_proxyauth(void) {
    return proxy_upwd;
}

hash_t * wsman_options_get_properties (void)
{
    int c = 0;
    hash_t *h = hash_create(HASHCOUNT_T_MAX, 0, 0);
    
    while(properties != NULL && properties[c] != NULL)
    {
        char *cc[3]; 
        u_tokenize1(cc, 2, properties[c], '=');
        printf("key=%s, value=%s", cc[0], cc[1]);
        if (!hash_alloc_insert(h, cc[0], cc[1])) {
            debug("hash_alloc_insert failed");
        }
        c++;
    }
    return h;
}   

int wsman_options_get_action (void)
{
    int op = 0;
    int i;
    for(i = 0; action_data[i].action != NULL; i++)
    {
        if (strcmp(action_data[i].action, _action)  == 0 ) {
            op = action_data[i].value;
            break;
        }
    }
    return op;
}   

char* wsman_options_get_resource_uri (void)
{	
    return resource_uri;
}   

int wsman_options_get_max_elements (void)
{	
    return enum_max_elements;
}   
char wsman_options_get_optimize_enum (void)
{	
    return enum_optimize;
}   
char wsman_options_get_estimate_enum (void)
{	
    return enum_estimate;
}   
char wsman_options_get_dump_request (void)
{	
    return dump_request;
}   

char * wsman_options_get_test_file (void)
{	
    return test_case;
}   
char * wsman_options_get_enum_mode (void)
{	
    return enum_mode;
}   
char * wsman_options_get_binding_enum_mode (void)
{	
    return binding_enum_mode;
}   
char * wsman_options_get_cim_namespace (void)
{	
    return cim_namespace;
}   
char * wsman_options_get_fragment (void)
{	
    return fragment;
}   
char * wsman_options_get_filter (void)
{	
    return wsm_filter;
}   
char * wsman_options_get_dialect (void)
{	
    return wsm_dialect;
}   
char * wsman_options_get_path (void)
{	
    return url_path;
}   
char * wsman_options_get_agent (void)
{	
    if (agent)
        return agent;
    else
        return PACKAGE_STRING;
}   
char * wsman_options_get_auth_method (void)
{	
    return authentication_method;
}

int wsman_options_get_no_verify_peer (void)
{
    return no_verify_peer;
}



