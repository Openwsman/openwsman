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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glib.h>

#include "ws_utilities.h"


#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "wsman-client-options.h"
#include "wsman.h"

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
static gchar *resource_uri = NULL;
static gchar *invoke_method = NULL;
static gchar **properties = NULL;

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
        { "password", 'p', 0, G_OPTION_ARG_STRING, &password, "Password", "<password>" },
        { "hostname", 'h', 0, G_OPTION_ARG_STRING, &server, "Host name", "<hostname>" },
        { "port", 'P', 0, G_OPTION_ARG_INT, &server_port, "Server Port", "<port>" },                
        { "method", 'a', 0, G_OPTION_ARG_STRING, &invoke_method, "Method (Works only with 'invoke')", "<custom method>" },                
        { "prop", 'k', 0, G_OPTION_ARG_STRING_ARRAY, &properties, "Properties with key value pairs (For 'put', 'invoke' and 'create')" , "<key=val>" },       
        { "config-file",	'C', 0, G_OPTION_ARG_FILENAME, 	&config_file,  	"Alternate configuration file", "<file>" },
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

    if (!wsman_read_client_config()) {
        fprintf(stderr, "Configuration file not found\n");
        return FALSE;
    }


    g_option_context_free(opt_ctx);
    return retval;
}

const char * wsman_options_get_config_file (void) {
    if (config_file != NULL && !g_path_is_absolute (config_file)) {
        char cwd[PATH_MAX];
        char *new_config_file;

        getcwd (cwd, PATH_MAX);
          
        new_config_file = g_strconcat (cwd, "/", config_file, NULL);

        g_free (config_file);
        config_file = new_config_file;
    }
    return config_file;
}

int wsman_read_client_config (void)
{
    GKeyFile *cf;
    char *filename;
    filename = (char *)wsman_options_get_config_file();
    if (!filename) 
        filename = DEFAULT_CONFIG_FILE;
    cf = g_key_file_new ();
    if (g_key_file_load_from_file (cf, filename, G_KEY_FILE_NONE, NULL))
    {
        if (g_key_file_has_group (cf, "client"))
        {
            
            if (g_key_file_has_key (cf, "client", "agent", NULL))
                agent = g_key_file_get_string (cf, "client", "agent", NULL);
        }
    } else {
        return 0;
    }
    g_key_file_free (cf);
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

GList * wsman_options_get_properties (void)
{
    int c = 0;
    
    GList *list = NULL;
    while(properties != NULL && properties[c] != NULL)
    {
        WsProperties *p =  malloc(sizeof(WsProperties));
        char **cc = g_strsplit(properties[c], "=", 2 );
        p->key = cc[0];
        p->value = cc[1];
        list=g_list_append(list, p);
        c++;
    }
    return list;
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
char * wsman_options_get_agent (void)
{	
    if (agent)
        return agent;
    else
        return PACKAGE_NAME;
}   
