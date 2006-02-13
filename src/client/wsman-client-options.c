
#include <stdio.h>

#include <glib.h>

#include "wsman-client-options.h"
#include "wsman.h"

static const char **wsman_argv = NULL;

static gint server_port =  -1;
static gchar *cafile = NULL;
static gint debug_level = -1;
static gint syslog_level = -1;

static gchar *username = NULL;
static gchar *password = NULL;
static gchar *server = NULL;

static gchar *get_action = NULL;
static gchar *enum_action = NULL;
static gchar *put_action = NULL;
static gchar *catch_action = NULL;
static gchar **properties = NULL;

gboolean wsman_parse_options(int argc, char **argv) 
{

	gboolean retval = FALSE;
	GError *error = NULL;
	
    GOptionEntry options[] = {						
    		{ "cafile", 'C', 0, G_OPTION_ARG_FILENAME, &cafile, "Certificate file", "<filename>"  },                          
        { "username", 'u', 0, G_OPTION_ARG_STRING, &username, "User name", "<username>" },
		{ "password", 'p', 0, G_OPTION_ARG_STRING, &password, "Password", "<password>" },
		{ "hostname", 'h', 0, G_OPTION_ARG_STRING, &server, "Host name", "<hostname>" },
        { "port", 'P', 0, G_OPTION_ARG_INT, &server_port, "Port", "<port>" },                
        { "prop", 'k', 0, G_OPTION_ARG_STRING_ARRAY, &properties, "Properties with key value pairs" , "<key=val>" },
        { NULL }
    };
    
   	GOptionEntry action_options[] = 
   	{				
		{ "get", 0, 0, G_OPTION_ARG_STRING, &get_action, "Transfer Get", "<Resource URI>"  },
		{ "put", 0, 0, G_OPTION_ARG_STRING, &put_action, "Transfer Put", "<Resource URI>"  },
		{ "enumerate", 0, 0, G_OPTION_ARG_STRING, &enum_action, "Enumeration", "<Resource URI>"  },
        { "catch", 0, 0, G_OPTION_ARG_STRING, &catch_action, "Private Catch (For Testing Only)", "<Resource URI>" },
		
		{ NULL }
   	};
   
   	GOptionGroup *action_group;
	GOptionContext *opt_ctx;	
    opt_ctx = g_option_context_new("WS-Management Client");    
    action_group = g_option_group_new("actions", "Actions", "Supported Actions", NULL, NULL);
    g_option_group_add_entries(action_group, action_options);
    
    g_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    g_option_context_add_main_entries(opt_ctx, options, "wsman");
    g_option_context_add_group(opt_ctx, action_group);
    
    retval = g_option_context_parse(opt_ctx, &argc, &argv, &error);
    
    if (error)
    {
    		if (error->message)
        		printf ("%s\n", error->message);
        return FALSE;
     }
                    
    g_option_context_free(opt_ctx);
    return retval;
}
        
const char **
wsman_options_get_argv (void)
{
    return wsman_argv;
}


int
wsman_options_get_debug_level (void)
{
    return debug_level;
}


int
wsman_options_get_syslog_level (void)
{
    return syslog_level;
}


int
wsman_options_get_server_port (void)
{
    return server_port;
}


char*
wsman_options_get_cafile (void)
{
    return cafile;
}        

char*
wsman_options_get_server (void)
{
    return server;
}   

char*
wsman_options_get_username (void)
{
    return username;
}   

char*
wsman_options_get_password (void)
{
    return password;
}  

int
wsman_options_get_action (void)
{
	int op;
	if (get_action != NULL)	
		op = ACTION_TRANSFER_GET;			
	else if (put_action != NULL)	
		op = ACTION_TRANSFER_PUT;			
	else if (enum_action != NULL)	
		op = ACTION_ENUMERATION;						
    else if (catch_action != NULL)    
        op = ACTION_PRIVATE_CATCH;
    else
    		op = 0;
    return op;
}   

char*
wsman_options_get_resource_uri (void)
{	
	char* resourceUri = NULL;
	if (get_action != NULL)	
		resourceUri = get_action;			
	else if (put_action != NULL)	
		resourceUri = put_action;					
	else if (enum_action != NULL)	
		resourceUri = enum_action;								
    else if (catch_action != NULL)    
        resourceUri = catch_action;        		
    return resourceUri;
}   


