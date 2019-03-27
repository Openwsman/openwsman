/*
 *  Copyright (c) 2006 Dell, Inc.
 *  by Praveen K Paladugu <praveen_paladugu@dell.com>
 *  Licensed under the GNU General Public license, version 2.
 */


#include "wsman_config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "u/libu.h"

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"
#include "wsman-client-api.h"

#include "redirect.h"

#include "../../server/wsmand-daemon.h"

struct __Redirect_Data
{
    char *server; 
    char *username;
    char *password;
    char *url_path;
    char *authentication_method;
    char *cim_namespace;
    char *cainfo, *sslkey, *cl_cert;
    char *namespace;
    int noverifypeer, noverifyhost;
    int server_port;

};

static struct __Redirect_Data *redirect_data =NULL;

SER_START_ITEMS(Redirect)
SER_END_ITEMS(Redirect);

/*As the data value in endPoints is not used, setting it to NULL for now*/
START_END_POINTS(Redirect)
    END_POINT_TRANSFER_DIRECT_GET(Redirect, NULL),
    END_POINT_TRANSFER_DIRECT_PUT(Redirect, NULL),
    END_POINT_TRANSFER_DIRECT_CREATE(Redirect, NULL),
    END_POINT_TRANSFER_DIRECT_DELETE(Redirect, NULL),
    END_POINT_ENUMERATE(Redirect, NULL),
    END_POINT_DIRECT_PULL(Redirect, NULL),
    END_POINT_PULL(Redirect, NULL),
    END_POINT_RELEASE(Redirect, NULL),
    END_POINT_CUSTOM_METHOD(Redirect, NULL),
FINISH_END_POINTS(Redirect);


static list_t *
set_namespaces(void) 
{

    int i;

    list_t *l = list_create(LISTCOUNT_T_MAX);
    WsSupportedNamespaces *ns = (WsSupportedNamespaces *)u_zalloc(
			sizeof(WsSupportedNamespaces));

    ns->class_prefix = NULL;
    ns->ns = redirect_data->namespace;
    lnode_t *node = lnode_create(ns);
    list_append(l, node);
    return l;
}


void get_endpoints(void *self, void **data) 
{		 		
    WsDispatchInterfaceInfo *ifc =(WsDispatchInterfaceInfo *)data;	
    ifc->flags = 0;
    ifc->actionUriBase = NULL;
    ifc->version = "2.2";
    ifc->vendor = "Dell";
    ifc->displayName = "Redirect";
    ifc->notes = "Redirect Plugin";
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanResourceUri = NULL;
    ifc->extraData = NULL;
    ifc->namespaces = set_namespaces();
    ifc->endPoints = Redirect_EndPoints;			
}


static int init( void *self, void **data )
{
    char* filename;
    dictionary *ini=NULL, *inc_ini=NULL;
    filename = (char *) wsmand_options_get_config_file();
    ini = iniparser_new(filename);

    if (ini == NULL) {
      error("Redirect Plugin: iniparser_new failed");
      return 0;
    }

    redirect_data =  u_zalloc (sizeof(struct __Redirect_Data));
    if (redirect_data == NULL){
	error("Redirect Plugin: Failed while allocating memory for redirect_data");
	return 0;
    }

    /*Check if the conf file has the required fields populated.*/
    if ( iniparser_getstring(ini, "redirect:server", NULL) ==NULL ||
         iniparser_getstring(ini, "redirect:resource", NULL) ==NULL
	){
	    /*if the redirection details are not provided in the core config file, check for an include tag, and check file in the include tag*/

	    filename=iniparser_getstring(ini,"redirect:include",NULL);
	    if (filename == NULL) goto err_out;

	    inc_ini=iniparser_new(filename);
	    if (inc_ini == NULL) goto err_out;


	    if ( iniparser_getstring(inc_ini, ":server",NULL) != NULL &&
		 iniparser_getstring(inc_ini,":resource",NULL) != NULL )
		return 1; /*the inputs are fine */

	    err_out:
	    error("Redirect Plugin: The required inputs are not provided in the config file");
	    return 0;
	}
    if (ini != NULL)
	iniparser_free(ini);

    if (inc_ini != NULL)
	iniparser_free (inc_ini);	

    return 1;
}

static void cleanup( void  *self, void *data )
{
    free(redirect_data);
    return;
}

static void set_config( void *self, dictionary *config )
{
    if (config == NULL)
	return;
    char *inc_filename;
    dictionary *inc_ini;

/*Check for include tag first, if exists, only use the included file*/

    if ( (inc_filename=iniparser_getstring(config,"redirect:include",NULL)) != NULL ){
  	
        inc_ini = iniparser_new(inc_filename);
	redirect_data->server = iniparser_getstr (inc_ini, ":server");
	redirect_data->namespace = iniparser_getstr (inc_ini, ":resource");

	redirect_data->username = iniparser_getstring (inc_ini, ":username",NULL);
	redirect_data->password = iniparser_getstring (inc_ini, ":password",NULL);
	redirect_data->url_path = iniparser_getstring (inc_ini, ":url_path","/wsman");
	redirect_data->authentication_method = iniparser_getstring (inc_ini, ":authentication_method", "basic");
	redirect_data->cim_namespace = iniparser_getstring (inc_ini, ":cim_namespace","root/cimv2");
	redirect_data->cainfo = iniparser_getstring (inc_ini, ":cacert",NULL);
	redirect_data->server_port = iniparser_getint (inc_ini, ":port",5895);
	redirect_data->noverifypeer = iniparser_getint (inc_ini, ":noverifypeer", 0);
	redirect_data->noverifyhost = iniparser_getint (inc_ini, ":noverifyhost", 0);
	redirect_data->sslkey = iniparser_getstring (inc_ini, ":sslkey", NULL);
	redirect_data->cl_cert = iniparser_getstring (inc_ini, ":cl_cert", NULL);		
    return;
    }

    

/*No Include file, read the main configuration file */
    redirect_data->server = iniparser_getstr (config, "redirect:server");
    redirect_data->namespace = iniparser_getstr (config, "redirect:resource");

    redirect_data->username = iniparser_getstring (config, "redirect:username",NULL);
    redirect_data->password = iniparser_getstring (config, "redirect:password",NULL);
    redirect_data->url_path = iniparser_getstring (config, "redirect:url_path","/wsman");
    redirect_data->authentication_method = iniparser_getstring (config, "redirect:authentication_method", "basic");
    redirect_data->cim_namespace = iniparser_getstring (config, "redirect:cim_namespace","root/cimv2");
    redirect_data->cainfo = iniparser_getstring (config, "redirect:cacert",NULL);
    redirect_data->server_port = iniparser_getint (config, "redirect:port",5895);
    redirect_data->noverifypeer = iniparser_getint (config, "redirect:noverifypeer", 0);
    redirect_data->noverifyhost = iniparser_getint (config, "redirect:noverifyhost", 0);
    redirect_data->sslkey = iniparser_getstring (config, "redirect:sslkey", NULL);
    redirect_data->cl_cert = iniparser_getstring (config, "redirect:cl_cert", NULL);		

	
}

static char *get_remote_cl_cert()
{
	return redirect_data->cl_cert;
}
static char *get_remote_sslkey()
{
	return redirect_data->sslkey;
}

static char* get_remote_server()
{
	return redirect_data->server;

}
static int get_remote_noverifypeer()
{
	return redirect_data->noverifypeer;
}
static int get_remote_noverifyhost()
{
	return redirect_data->noverifyhost;
}
static char* get_remote_username()
{
	return redirect_data->username;
}

static char* get_remote_password()
{
	return redirect_data->password;

}

static char* get_remote_url_path()
{
	return redirect_data->url_path;
}

static char* get_remote_namespace()
{
	return redirect_data->namespace;
}


static char* get_remote_authentication_method()
{
	return redirect_data->authentication_method;
}

static char* get_remote_cim_namespace()
{
	return redirect_data->cim_namespace;
}

static char* get_remote_cainfo()
{
	return redirect_data->cainfo;
}

static int get_remote_server_port()
{
	return redirect_data->server_port;
}


WsManClient* setup_redirect_client(WsContextH cntx, char *ws_username, char *ws_password)
{
	
    WsManClient *cl = NULL;

	cl = wsmc_create(
		get_remote_server() ,
                get_remote_server_port() ,
                get_remote_url_path(),
                get_remote_cainfo() ? "https" : "http",
		/* wsmc_create duplicates the username/password passed, no need to duplicate again. */
                get_remote_username() ? get_remote_username() : ws_username,
                get_remote_password() ? get_remote_password() : ws_password 
         );

    if (cl == NULL){
	error("Redirect Plugin: Error while creating the client for redirection");
	return NULL;
    }


	wsman_transport_set_auth_method(cl, get_remote_authentication_method());

        if ( get_remote_cainfo() ) {
                wsman_transport_set_cainfo(cl, get_remote_cainfo() );
        }

	if (get_remote_cl_cert()){
		wsman_transport_set_cert(cl, get_remote_cl_cert());
		if (!get_remote_cainfo())
                        debug("Warning: cainfo not set to enable SSL operation in Redirect Plugin\n");

	}

        if ( get_remote_sslkey())
        {
		wsman_transport_set_cert(cl, get_remote_sslkey());
		if (!get_remote_cainfo())
		{
			debug("Warning: cainfo not set to enable SSL operation in Redirect Plugin\n");
		}
	}


        wsman_transport_set_verify_peer(cl, get_remote_cainfo()? !get_remote_noverifypeer() : 0);
        wsman_transport_set_verify_host(cl, get_remote_cainfo() ? !get_remote_noverifyhost(): 0 );




    return cl; 
}

