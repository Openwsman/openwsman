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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "ws_utilities.h"


#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "wsman-client-options.h"
#include "wsman.h"

static const char **wsman_argv = NULL;

static gint server_port =  -1;
static gchar *cafile = NULL;
static gint debug_level = -1;
static gint enum_max_elements = 0;

static gchar *username = NULL;
static gchar *password = NULL;
static gchar *server = NULL;

static gchar *_action = NULL;
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
        { "port", 'P', 0, G_OPTION_ARG_INT, &server_port, "Port", "<port>" },                
        { "method", 'a', 0, G_OPTION_ARG_STRING, &invoke_method, "Method (Works only with 'invoke')", "<custom method>" },                
        { "prop", 'k', 0, G_OPTION_ARG_STRING_ARRAY, &properties, "Properties with key value pairs (For 'put', 'invoke' and 'create')" , "<key=val>" },       
        { NULL }
    };
    GOptionEntry enum_options[] = 
    {				
        { "max-elements", 'e', 0, G_OPTION_ARG_INT, &enum_max_elements, "Max Elements Per Pull", "<max number of elements>"  },
#ifdef DMTF_SPEC_1        
        { "optimize", 'o', 0, G_OPTION_ARG_NONE, &enum_optimize, "Optimize enumeration results", NULL  },
#endif
        { NULL }
    };
    GOptionGroup *enum_group;

    GOptionContext *opt_ctx;	
    opt_ctx = g_option_context_new("<action> <Resource Uri>");    
    enum_group = g_option_group_new("enumeration", "Enumeration", "Enumeration Options", NULL, NULL);
    g_option_group_add_entries(enum_group, enum_options);

    g_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    g_option_context_add_main_entries(opt_ctx, options, "wsman");
    g_option_context_add_group(opt_ctx, enum_group);

    retval = g_option_context_parse(opt_ctx, &argc, &argv, &error);

    if (argc > 2 ) {
        _action = argv[1];
        resource_uri = argv[2];
    } else {
        if (argc == 1 && strcmp(argv[1], "identify") != 0 ) {
            _action = argv[1];
        }
        fprintf(stderr, "Error: operation can not be completed. Action or/and Resource Uri missing.\n");
        return FALSE;
    }


    if (error) {
        if (error->message)
            printf ("%s\n", error->message);
        return FALSE;
    }

    g_option_context_free(opt_ctx);
    return retval;
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

