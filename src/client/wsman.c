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
 * @author Eugene Yarmosh
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#ifdef PRETTY_OUTPUT
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#endif

#include <glib.h>
#include "libsoup/soup.h"
#include "libsoup/soup-session.h"



#include "ws_utilities.h"


#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"


#include "wsman.h"




SoapH wsman_client_initialize() {
    SOAP_FW *fw = NULL;

    fw = (SOAP_FW*)soap_alloc(sizeof(SOAP_FW), 1);
    if ( fw )
    {
        fw->dispatchList.listOwner = fw;
        fw->cntx = ws_create_context((SoapH)fw);
        soap_initialize_lock(fw);
        ws_xml_parser_initialize((SoapH)fw, g_wsNsData);
        //SoapAddDefaltFilter((SoapH)fw, OutboundAddressingFilter, NULL, 0);
    }
    return (SoapH)fw;
}

WsContextH wsman_client_create() {
    SoapH soap = wsman_client_initialize();
    return ((SOAP_FW*)soap)->cntx;
}



int main(int argc, char** argv)
{     
    int retVal = 0;   
	int op=0;

    WsClientContextH *ctx;

    if((ctx = (WsClientContextH*)malloc(sizeof(struct _WsClientContext *))) == NULL)
    {
        fprintf(stderr, "Could not malloc\n");
        exit (1);
    }

    g_type_init ();
    g_thread_init (NULL);
    
    
    GError *error = NULL;
    gboolean retval;
    gchar *cafile = NULL;
    gchar *username = NULL;
    gchar *password = NULL;
    gchar *server = NULL;
    
    gint port = 0;
    gchar **prop = NULL;
    
    GOptionEntry options[] = {
						
                { "cafile", 'C', 0, G_OPTION_ARG_FILENAME, &cafile, "Certificate file", "<filename>"  },                          
                { "username", 'u', 0, G_OPTION_ARG_STRING, &username, "User name", "<username>" },
				{ "password", 'p', 0, G_OPTION_ARG_STRING, &password, "Password", "<password>" },
				{ "hostname", 'h', 0, G_OPTION_ARG_STRING, &server, "Host name", "<hostname>" },
                { "port", 'P', 0, G_OPTION_ARG_INT, &port, "Port", "<port>" },                
                { "prop", 'k', 0, G_OPTION_ARG_STRING_ARRAY, &prop, "Properties with key value pairs" , "<key=val>" },
                { NULL }
        };
 
    gchar *get_action = NULL;
    gchar *enum_action = NULL;
    gchar *put_action = NULL;
    gchar *catch_action = NULL;
    
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
        return 1;
     }
        
    g_option_context_free(opt_ctx);
        	
    char *_ep_uri = g_strdup_printf("http://%s:%d/%s",  server, port, "wsman");
    
    WsContextH cntx = wsman_client_create();
    ctx->wscntx =  cntx;    
    ctx->username =  username;
    ctx->password =  password;


	if (get_action != NULL)
		op = ACTION_TRANSFER_GET;
	else if (put_action != NULL)
		op = ACTION_TRANSFER_PUT;
	else if (enum_action != NULL)
		op = ACTION_ENUMERATION;			
    else if (catch_action != NULL)
        op = ACTION_PRIVATE_CATCH;

	   
	switch (op) 
	{
		case  ACTION_TRANSFER_GET: 
        		retVal = wsman_transfer_get(_ep_uri, ctx, get_action);
        break;
   		case ACTION_ENUMERATION:
        		retVal = wsman_enumeration(_ep_uri, ctx, enum_action ,  5);
		break;
        case ACTION_PRIVATE_CATCH:
            retVal = wsman_private_catch(_ep_uri, ctx, catch_action);
        break;
  		default:
    			fprintf(stderr, "Action not supported\n");    		
    			retVal = 1;
	}

    g_free(_ep_uri);
    
    soap_free(cntx);
    return retVal;
}
