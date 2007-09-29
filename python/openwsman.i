%module pywsman
%{
#include "wsman-api.h"
#include "wsman-names.h"
#include "openwsman.h"
%}


#define FLAG_NONE                            0x0000
#define FLAG_ENUMERATION_COUNT_ESTIMATION    0x0001
#define FLAG_ENUMERATION_OPTIMIZATION        0x0002
#define FLAG_ENUMERATION_ENUM_EPR            0x0004
#define FLAG_ENUMERATION_ENUM_OBJ_AND_EPR    0x0008
#define FLAG_DUMP_REQUEST                    0x0010
#define FLAG_IncludeSubClassProperties       0x0020
#define FLAG_ExcludeSubClassProperties       0x0040
#define FLAG_POLYMORPHISM_NONE               0x0080
#define FLAG_MUND_MAX_ESIZE                  0x0100
#define FLAG_MUND_LOCALE                     0x0200
#define FLAG_MUND_OPTIONSET                  0x0400
#define FLAG_MUND_FRAGMENT                   0x0800
#define FLAG_CIM_EXTENSIONS                  0x1000
#define FLAG_CIM_REFERENCES                  0x2000
#define FLAG_CIM_ASSOCIATORS                 0x4000
#define FLAG_EVENT_SENDBOOKMARK		0X8000



/* options */
client_opt_t *wsmc_options_init(void);
void wsmc_options_destroy(client_opt_t * op);

void wsmc_set_filter(const char *filter, client_opt_t * options);

void wsmc_set_dialect(const char *dialect, client_opt_t * options);
 
void _set_assoc_filter(client_opt_t *options);



WsManClient *wsmc_create_from_uri(const char *endpoint);

WsManClient *wsmc_create(const char *hostname,
                                         const int port, const char *path,
                                         const char *scheme,
                                         const char *username,
                                         const char *password);

void wsmc_release(WsManClient * cl);

char *_identify(WsManClient * cl, client_opt_t * options, char *encoding);

char *_pull(WsManClient * cl, const char *resource_uri, 
	client_opt_t * options, const char *enumContext, char *encoding);
	
char *_release(WsManClient * cl, const char *resource_uri, client_opt_t * options, 
	const char *enumContext, char *encoding);

char *_enumerate(WsManClient * cl, const char *resource_uri, client_opt_t * options,  char *encoding);

char *_get(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);

char *_delete(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);

char *_invoke(WsManClient * cl, const char *resource_uri, client_opt_t * options, 
	const char *method, const char *data, size_t size, char *encoding);

char *_put(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *data, size_t size, char *encoding);

char *_subscribe(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);

char *_renew(WsManClient *cl, client_opt_t *options, char *uuid, char *encoding);

char *_unsubscribe(WsManClient *cl, client_opt_t *op, char *uuid, char *encoding);

void wsmc_set_dumpfile( WsManClient *cl, FILE *f );

void wsmc_add_selector(client_opt_t * options, const char *key, const char *value);

long wsmc_get_response_code(WsManClient * cl);

void wsmc_set_action_option(client_opt_t * options,
				     unsigned int);

