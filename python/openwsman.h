

void init_pywsman(void);
char *_identify(WsManClient * cl, client_opt_t * options, char *encoding);
char *_pull(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *enumContext, char *encoding);
char *_release(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *enumContext, char *encoding);
char *_enumerate(WsManClient * cl, const char *resource_uri, client_opt_t * options,  char *encoding);
char *_get(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);
void _set_assoc_filter(client_opt_t *options);
char *_invoke(WsManClient * cl, const char *resource_uri, client_opt_t * options,
                const char *method, const char *data, size_t size, char *encoding);
char *_delete(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);

char *_put(WsManClient * cl, const char *resource_uri, client_opt_t * options,
                const char *data, size_t size, char *encoding);

char *_subscribe(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);

char *_renew(WsManClient *cl, client_opt_t *options, char *uuid, char *encoding);

char *_unsubscribe(WsManClient *cl, client_opt_t *op, char *uuid, char *encoding);
