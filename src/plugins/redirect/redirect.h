/*
 *  Copyright (c) 2006 Dell, Inc.
 *  by Praveen K Paladugu <praveen_paladugu@dell.com>
 *  Licensed under the GNU General Public license, version 2.
 */


#include "wsman-declarations.h"
#include "wsman-xml-serializer.h"
#include "wsman-client-transport.h"

#define XML_REDIRECT_NS    "http://dummy.com/wbem/wscim/1/cim-schema/2"


struct  __RedirectResult
{
	int result;
};
typedef struct __RedirectResult Redirect;

// Service endpoint declaration
int Redirect_Enumerate_EP(WsContextH cntx,
                        WsEnumerateInfo* enumInfo,
                        WsmanStatus *status, void *opaqueData);


int Redirect_Pull_EP(WsContextH cntx, WsEnumerateInfo* enumInfo, 
		    WsmanStatus *status, void *opaqueData);


int Redirect_Get_EP( SoapOpH op, void* appData, void *opaqueData );

int Redirect_Create_EP( SoapOpH op, void* appData, void *opaqueData );

int Redirect_Delete_EP( SoapOpH op, void* appData, void *opaqueData );

int Redirect_Put_EP( SoapOpH op, void* appData, void *opaqueData );

int Redirect_Custom_EP( SoapOpH op, void* appData, void *opaqueData );

int Redirect_transfer_action ( SoapOpH op, void* appData, void *opaqueData);

int Redirect_Release_EP(WsContextH cntx,
                        WsEnumerateInfo* enumInfo,
                        WsmanStatus *status, void *opaqueData);

/*int Redirect_Subscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int Redirect_Renew_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int Redirect_UnSubscribe_EP(WsContextH cntx,
		WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int Redirect_Evt_Pull_EP(WsContextH cntx,WsSubscribeInfo* subsInfo,
		WsmanStatus *status,
		void *opaqueData);

int Redirect_EventPoll_EP(WsEventThreadContextH threadcntx);
*/





SER_DECLARE_TYPE(Redirect);
DECLARE_EP_ARRAY(Redirect);


void get_endpoints(void *self, void **data);
//int init (void *self, void **data );
//void cleanup( void *self, void *data );
//void set_config( void *self, dictionary *config );

static char* get_remote_server();
static char* get_remote_username();
static char* get_remote_password();
static char* get_remote_url_path();
static char* get_remote_authentication_method();
static char* get_remote_cim_namespace();
static char* get_remote_cainfo();
static int get_remote_server_port();
static int get_remote_noverifypeer();
static int get_remote_noverifyhost();
static char* get_remote_sslkey();
static char* get_remote_cl_cert();

WsManClient* setup_redirect_client (WsContextH cntx, char *username, char *password);


