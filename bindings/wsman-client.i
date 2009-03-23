/*
 * wsman-client.i
 *
 * client declarations for openwsman swig bindings
 *
 * WsManClient
 *
 */
 
%rename(Client) _WsManClient;
%nodefault _WsManClient;
typedef struct _WsManClient {
} WsManClient;

%extend WsManClient {
  /* constructor */
  WsManClient( const char *uri ) {
    return wsmc_create_from_uri( uri );
  }
  WsManClient(const char *hostname,
              const int port, const char *path,
              const char *scheme,
              const char *username,
              const char *password) {
    return wsmc_create( hostname, port, path, scheme, username, password );
  }
  /* destructor */
  ~WsManClient() {
    wsmc_release( $self );
  }
  /* set dumpfile */
#if defined(SWIGRUBY)
  %rename( "dumpfile=" ) set_dumpfile( FILE *f );
#endif
  void set_dumpfile( FILE *f ) {
    wsmc_set_dumpfile( $self, f );
  }
  long response_code() {
    return wsmc_get_response_code( $self );
  }
  char *scheme() {
    return wsmc_get_scheme( $self );
  }
  char *host() {
    return wsmc_get_hostname( $self );
  }
  int port() {
    return wsmc_get_port( $self );
  }
  char *path() {
    return wsmc_get_path( $self );
  }
  char *user() {
    return wsmc_get_user( $self );
  }
  char *password() {
    return wsmc_get_password( $self );
  }
  
  /*
   * transport
   */
  WsManTransport *transport() {
    wsmc_transport_init($self, NULL);
    wsmc_transport_set_auth_request_func( $self, auth_request_callback );

    return (WsManTransport *)$self;
  }

  int send_request(WsXmlDocH request) {
    return wsman_send_request($self, request);
  }



/*-----------------------------------------------------------------*/
/* actions */

  WsXmlDocH identify( client_opt_t *options ) {
    return wsmc_action_identify( $self, options );
  }
  
  WsXmlDocH get_from_epr( client_opt_t *options , epr_t *epr) {
    return wsmc_action_get_from_epr( $self, epr, options);
  }
  WsXmlDocH delete_from_epr( client_opt_t *options , epr_t *epr) {
    return wsmc_action_delete_from_epr( $self, epr, options);
  }

  WsXmlDocH enumerate( client_opt_t *options , filter_t *filter, char *resource_uri) {
    return wsmc_action_enumerate( $self, resource_uri, options, filter);
  }
  WsXmlDocH pull( client_opt_t *options , filter_t *filter, char *resource_uri, char *enum_ctx) {
    return wsmc_action_pull( $self, resource_uri, options, filter, enum_ctx);
  }
  WsXmlDocH create( client_opt_t *options, char *resource_uri, char *data, size_t size, char *encoding) {
    return wsmc_action_create_fromtext( $self, resource_uri, options, data, size, encoding);
  }
  WsXmlDocH put( client_opt_t *options , char *resource_uri,  char *data, size_t size, char *encoding) {
    return wsmc_action_put_fromtext( $self, resource_uri, options, data, size, encoding);
  }
  WsXmlDocH release( client_opt_t *options , char *resource_uri, char *enum_ctx) {
    return wsmc_action_release( $self, resource_uri, options, enum_ctx);
  }
  WsXmlDocH get( client_opt_t *options , char *resource_uri) {
    return wsmc_action_get( $self, resource_uri, options);
  }
  WsXmlDocH delete( client_opt_t *options , char *resource_uri) {
    return wsmc_action_delete( $self, resource_uri, options);
  }
  WsXmlDocH invoke( client_opt_t *options , char *resource_uri, char *method, char *data = NULL, size_t size = 0, char *encoding = "utf-8") {
    return wsmc_action_invoke_fromtext( $self, resource_uri, options, method, data, size, encoding);
  }

  WsXmlDocH subscribe(client_opt_t *options , filter_t *filter, char *resource_uri) {
    return wsmc_action_subscribe($self,  resource_uri, options, filter);
  }

  WsXmlDocH unsubscribe(client_opt_t *options , filter_t *filter, char *resource_uri, char *identifier) {
    return wsmc_action_unsubscribe($self, resource_uri, options, identifier);
  }

  WsXmlDocH renew(client_opt_t *options , char *resource_uri, char *identifier) {
    return wsmc_action_renew($self, resource_uri, options, identifier);
  }

}
