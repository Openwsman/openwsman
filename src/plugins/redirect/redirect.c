
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

#include "redirect.h"


char *server; 
char *username;
char *password;
char  *url_path= "/wsman";
char  *authentication_method="basic";
char  *cim_namespace="root/cimv2";
char *cainfo=NULL, *sslkey=NULL, *cl_cert=NULL;
char *namespace;
int noverifypeer, noverifyhost;

int server_port=5895;

SER_START_ITEMS(Redirect)
SER_END_ITEMS(Redirect);

//As the data value in endPoints is not used, setting it to NULL for now
START_END_POINTS(Redirect)
    END_POINT_TRANSFER_GET(Redirect, NULL),
    END_POINT_ENUMERATE(Redirect, NULL),
    END_POINT_DIRECT_PULL(Redirect, NULL),
    END_POINT_PULL(Redirect, NULL),
    END_POINT_RELEASE(Redirect, NULL),
    END_POINT_TRANSFER_PUT(Redirect, NULL),
FINISH_END_POINTS(Redirect);


//START_NAMESPACES(Redirect)
//    ADD_NAMESPACE( XML_REDIRECT_NS, NULL ),
//FINISH_NAMESPACES(Redirect);

static list_t *
set_namespaces(void) 
{

  int i;

    list_t *l = list_create(LISTCOUNT_T_MAX);
    WsSupportedNamespaces *ns = (WsSupportedNamespaces *)u_malloc(sizeof(WsSupportedNamespaces));
    ns->class_prefix = NULL;
    ns->ns = namespace;
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


int init( void *self, void **data )
{
  char* filename;
  dictionary *ini;
        filename = (char *) wsmand_options_get_config_file();
        ini = iniparser_new(filename);
    if ( iniparser_getstring(ini, "redirect:server", NULL) ==NULL ||
         iniparser_getstring(ini, "redirect:namespace", NULL) ==NULL
	){
	error("The required inputs are not provided in the config file");
	return 0;
   }
    	
    return 1;
}

void
cleanup( void  *self, void *data )
{
//TODO:Free up the memory allocated for the global vars
	return;
}
void set_config( void *self, dictionary *config )
{
    debug("Reading the redirection info from config file");
    if (config) {
     server = iniparser_getstr (config, "redirect:server");
     namespace = iniparser_getstr (config, "redirect:namespace");

     username = iniparser_getstring (config, "redirect:username",NULL);
     password = iniparser_getstring (config, "redirect:password",NULL);
     url_path = iniparser_getstring (config, "redirect:url_path","/wsman");
     authentication_method = iniparser_getstring (config, "redirect:authentication_method", "basic");
     cim_namespace = iniparser_getstring (config, "redirect:cim_namespace","root/cimv2");
     cainfo = iniparser_getstring (config, "redirect:cacert",NULL);
     server_port = iniparser_getint (config, "redirect:port",5895);
     noverifypeer = iniparser_getint (config, "redirect:noverifypeer", 0);
     noverifyhost = iniparser_getint (config, "redirect:noverifyhost", 0);	     sslkey = iniparser_getstring (config, "redirect:sslkey", NULL);
     cl_cert = iniparser_getstring (config, "redirect:cl_cert", NULL);		
    }		
}

char *get_remote_cl_cert()
{
	return cl_cert;
}
char *get_remote_sslkey()
{
	return sslkey;
}

char* get_remote_server()
{
	return server;

}
int get_remote_noverifypeer()
{
	return noverifypeer;
}
int get_remote_noverifyhost()
{
	return noverifyhost;
}
char* get_remote_username()
{
	return username;
}

char* get_remote_password()
{
	return password;

}

char* get_remote_url_path()
{
	return url_path;
}

char* get_remote_namespace()
{
	return namespace;
}


char* get_remote_authentication_method()
{
	return authentication_method;
}

char* get_remote_cim_namespace()
{
	return cim_namespace;
}

char* get_remote_cainfo()
{
	return cainfo;
}

int get_remote_server_port()
{
	return server_port;
}

