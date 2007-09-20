%module OpenWSMan
%{
#include "wsman-api.h"
#include "wsman-names.h"
%}





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
char *_identify(WsManClient * cl, client_opt_t * options, char *encoding);
char *_pull(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *enumContext, char *encoding);
char *_release(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *enumContext, char *encoding);
char *_enumerate(WsManClient * cl, const char *resource_uri, client_opt_t * options,  char *encoding);
char *_get(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);
char *_delete(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);
char *_invoke(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *method, const char *data, size_t size, char *encoding);

char *_put(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *data, size_t size, char *encoding);


void wsmc_add_selector(client_opt_t * options, const char *key, const char *value);

long wsmc_get_response_code(WsManClient * cl);

