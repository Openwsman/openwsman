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
#include <glib.h>

#include "u/libu.h"


#include "wsman-xml-api.h"
#include "wsman-errors.h"
#include "wsman-soap.h"
#include "wsman-client-options.h"
#include "wsman.h"

static char *auth_methods[] = {
     "basic",
     "digest",
     "ntlm",
     NULL,
};

static const char **wsman_argv = NULL;

static gint server_port =  80;
static gchar *cafile = NULL;
static gint debug_level = -1;
static gchar *test_case = NULL;
static gint enum_max_elements = 0;
gboolean  enum_optimize = FALSE;
gboolean  enum_estimate = FALSE;
gboolean  dump_request = FALSE;
static gchar *enum_mode = NULL;
static gchar *binding_enum_mode = NULL;
//gboolean  dump_response = FALSE;

static gchar *username = NULL;
static gchar *password = NULL;
static gchar *server = "localhost";
static gchar *cim_namespace = NULL;
static gchar *fragment = NULL;
static gchar *wsm_filter = NULL;
static gchar *wsm_dialect = NULL;

static gulong operation_timeout = 0;
static gulong max_envelope_size = 0;

static gchar *_action = NULL;
static gchar *agent = NULL;
static gchar *config_file = NULL;
static gchar *output_file = NULL;
static gchar *resource_uri = NULL;
static gchar *invoke_method = NULL;
static gchar *url_path = NULL;
static gchar **properties = NULL;
static gchar *authentication_method = NULL;
static gboolean no_verify_peer = FALSE;
static gchar *proxy = NULL;
static gchar *proxy_upwd = NULL;

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

gboolean wsman_parse_options(int argc, char **argv) 
{

    gboolean retval = FALSE;
    GError *error = NULL;

    GOptionEntry options[] = {
	    { "debug",'d', 0 ,G_OPTION_ARG_INT,&debug_level,"Set the verbosity of debugging output.", "1-6" },
        { "cafile", 'c', 0, G_OPTION_ARG_FILENAME, &cafile, "Certificate file", "<filename>"  },
        { "username", 'u', 0, G_OPTION_ARG_STRING, &username, "User name", "<username>" },
        { "path", 'g', 0, G_OPTION_ARG_STRING, &url_path, "Path", "<path>" },
        { "password", 'p', 0, G_OPTION_ARG_STRING, &password, "Password", "<password>" },
        { "hostname", 'h', 0, G_OPTION_ARG_STRING, &server, "Host name", "<hostname>" },
        { "port", 'P', 0, G_OPTION_ARG_INT, &server_port, "Server Port", "<port>" }, 
        { "proxy", 'X', 0, G_OPTION_ARG_STRING, &proxy, "Proxy name", "<proxy>" },
        { "proxyauth", 'Y', 0, G_OPTION_ARG_STRING, &proxy_upwd, "Proxy user:pwd", "<proxyauth>" },
        { "auth", 'y', 0, G_OPTION_ARG_STRING, &authentication_method, "Authentication Method", "<basic|digest>" },
        { "method", 'a', 0, G_OPTION_ARG_STRING, &invoke_method, "Method (Works only with 'invoke')", "<custom method>" },
        { "prop", 'k', 0, G_OPTION_ARG_STRING_ARRAY, &properties,
                    "Properties with key value pairs (For 'put', 'invoke' and 'create')" , "<key=val>" },
        { "config-file",	'C', 0, G_OPTION_ARG_FILENAME, 	&config_file,  	"Alternate configuration file", "<file>" },
        { "out-file",	'O', 0, G_OPTION_ARG_FILENAME, 	&output_file,  	"Write output to file", "<file>" },
        { "noverifypeer",  'V', 0, G_OPTION_ARG_NONE,  &no_verify_peer,   "Not to verify peer certificate", NULL },

        { NULL }
    };



    GOptionEntry request_options[] = 
    {				
        { "filter", 'x', 0, G_OPTION_ARG_STRING, &wsm_filter, "Filter" , "<filter>" }, 
        { "dialect", 'D', 0, G_OPTION_ARG_STRING, &wsm_dialect, "Filter Dialect" , "<dialect>" }, 
        { "timeout", 't', 0, G_OPTION_ARG_INT, &operation_timeout, "Operation timeout in seconds" , "<time in sec>" },       
        { "max-envelope-size", 'e', 0, G_OPTION_ARG_INT, &max_envelope_size, "maximal envelope size" , "<size>" },       
        { "fragment", 'F', 0, G_OPTION_ARG_STRING, &fragment, "Fragment (Supported Dialects: XPATH)" , "<fragment>" },       
        { NULL }
    };

    GOptionEntry enum_options[] = 
    {				
#ifdef DMTF_WSMAN_SPEC_1        
        { "max-elements", 'm', 0, G_OPTION_ARG_INT, &enum_max_elements, "Max Elements Per Pull/Optimized Enumeration", "<max number of elements>"  },
        { "optimize", 'o', 0, G_OPTION_ARG_NONE, &enum_optimize, "Optimize enumeration results", NULL  },
        { "estimate-count", 'E', 0, G_OPTION_ARG_NONE, &enum_estimate, "Return estimation of total items", NULL  },
        { "enum-mode", 'M', 0, G_OPTION_ARG_STRING, &enum_mode, "Enumeration Mode", "epr|objepr"  },
#endif
        { NULL }
    };
    
    GOptionEntry cim_options[] = 
    {				
#ifdef DMTF_WSMAN_SPEC_1        
        { "namespace", 'N', 0, G_OPTION_ARG_STRING, &cim_namespace, "CIM Namespace (default is root/cimv2)", "<namespace>"  },
        { "binding-enum-mode", 'B', 0, G_OPTION_ARG_STRING, &binding_enum_mode, "CIM binding Enumeration Mode", "none|include|exclude"  },
#endif
        { NULL }
    };

    GOptionEntry test_options[] = 
    {				
        { "from-file", 'f', 0, G_OPTION_ARG_FILENAME, &test_case, "Send request from file", "<file name>"  },
        { "print-request", 'R', 0, G_OPTION_ARG_NONE, &dump_request, "print request on stdout", NULL},
        //{ "print-response", 'N', 0, G_OPTION_ARG_NONE, &dump_response, "print all responses to stdout", NULL},
        { NULL }
    };

    GOptionGroup *enum_group;
    GOptionGroup *test_group;
    GOptionGroup *cim_group;
    GOptionGroup *req_flag_group;

    GOptionContext *opt_ctx;	
    opt_ctx = g_option_context_new("<action> <Resource Uri>");    
    enum_group = g_option_group_new("enumeration", "Enumeration", "Enumeration Options", NULL, NULL);
    test_group = g_option_group_new("tests", "Tests", "Test Cases", NULL, NULL);
    cim_group = g_option_group_new("cim", "CIM", "CIM Options", NULL, NULL);
    req_flag_group = g_option_group_new("flags", "Flags", "Request Flags", NULL, NULL);

    g_option_group_add_entries(enum_group, enum_options);
    g_option_group_add_entries(test_group, test_options);
    g_option_group_add_entries(cim_group, cim_options);
    g_option_group_add_entries(req_flag_group, request_options);

    g_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    g_option_context_add_main_entries(opt_ctx, options, "wsman");
    g_option_context_add_group(opt_ctx, enum_group);
    g_option_context_add_group(opt_ctx, test_group);
    g_option_context_add_group(opt_ctx, cim_group);
    g_option_context_add_group(opt_ctx, req_flag_group);

    retval = g_option_context_parse(opt_ctx, &argc, &argv, &error);

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


    if (error) {
        if (error->message)
            printf ("%s\n", error->message);
        return FALSE;
    }
    g_option_context_free(opt_ctx);
    return retval;
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
        server_port = iniparser_getint(ini, "client:port", 80);
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
gboolean wsman_options_get_optimize_enum (void)
{	
    return enum_optimize;
}   
gboolean wsman_options_get_estimate_enum (void)
{	
    return enum_estimate;
}   
gboolean wsman_options_get_dump_request (void)
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


int wsman_is_auth_method(int method)
{
    if (authentication_method == NULL) {
        return 1;
    }
    if (method >= AUTH_MAX) {
        return 0;
    }
    return (!strncasecmp(authentication_method, auth_methods[method],
            strlen(authentication_method)));
}

