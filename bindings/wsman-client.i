/*
 * wsman-client.i
 *
 * client declarations for openwsman swig bindings
 *
 */
 
%rename(Client) _WsManClient;
%nodefault _WsManClient;
typedef struct _WsManClient {
} WsManClient;

/*
 * Instances of Client represent a connection to a client used for
 * sending WS-Management operation requests.
 *
 */
 
%extend WsManClient {
  /*
   * Create a client connection.
   *
   * There are two ways to connect to a client, either by specifying a
   * URL or by passing all client parameters separately
   *
   * call-seq:
   *  Client.new(uri)
   *  Client.new(host, port, path, scheme, username, password)
   *
   *  Client.new("http://user:pass@host.domain.com:1234/path")
   *  Client.new("host.domain.com", 1234, "/path", "http", "user", "pass")
   *
   */
  WsManClient( const char *uri ) {
    return wsmc_create_from_uri( uri );
  }

  /*
   * :nodoc:
   */
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

  /*
   * Set the dumpfile (for debugging) to dump xml requests
   *
   * call-seq:
   *  client.dumpfile = File.open(...)
   *
   */
  void set_dumpfile( FILE *f ) {
    wsmc_set_dumpfile( $self, f );
  }

  /*
   * Response code of the last request (HTTP response code)
   *
   * call-seq:
   *  client.reponse_code -> Integer
   *
   */
  long response_code() {
    return wsmc_get_response_code( $self );
  }

  /*
   * String representation of the transport scheme ('http', 'https')
   */
  char *scheme() {
    return wsmc_get_scheme( $self );
  }

  /*
   * The host part of the client URL
   */
  char *host() {
    return wsmc_get_hostname( $self );
  }

  /*
   * The TCP port used in the connection
   */
  int port() {
    return wsmc_get_port( $self );
  }

  /*
   * The path of the clien URL
   */
  char *path() {
    return wsmc_get_path( $self );
  }

  /*
   * The user name used for authentication
   */
  char *user() {
    return wsmc_get_user( $self );
  }

  /*
   * The password used for authentication
   */
  char *password() {
    return wsmc_get_password( $self );
  }
  
  /*
   * The Transport instance associated to the client
   */
  WsManTransport *transport() {
    wsmc_transport_init($self, NULL);
    wsmc_transport_set_auth_request_func( $self, auth_request_callback );

    return (WsManTransport *)$self;
  }

  /*
   * Send a (raw) SOAP request to the client
   *
   * call-seq:
   *   result = client.send_request(XmlDoc.new("<xml ...>...</xml>"))
   *
   */
  int send_request(WsXmlDocH request) {
    return wsman_send_request($self, request);
  }

  /*
   * Get client encoding
   *
   * call-seq:
   *   client.encoding -> "utf-8"
   *
   */
  char *encoding() {
    return wsmc_get_encoding($self);
  }
  
#if defined(SWIGRUBY)
  %rename( "encoding=" ) set_encoding( const char *encoding );
#endif
  /*
   * Set client encoding
   *
   * call-seq:
   *   client.encoding = "utf-8"
   *
   */
  void set_encoding(const char *encoding) {
    wsmc_set_encoding($self, encoding);
  }

/*-----------------------------------------------------------------*/
/* actions */

  /*
   * WS-Identify
   *
   * identify: Sends an identify request
   *
   * call-seq:
   *   result = client.identify(options)
   *
   */
  WsXmlDocH identify( client_opt_t *options ) {
    return wsmc_action_identify( $self, options );
  }
  
  /*
   * WS-Get
   *
   * get_from_epr: Get a resource via an endpoint reference
   *
   */
  WsXmlDocH get_from_epr( client_opt_t *options , epr_t *epr) {
    return wsmc_action_get_from_epr( $self, epr, options);
  }

  /*
   * WS-Delete
   *
   * delete_from_epr: Remove a resource via an endpoint reference
   *
   */
  WsXmlDocH delete_from_epr( client_opt_t *options , epr_t *epr) {
    return wsmc_action_delete_from_epr( $self, epr, options);
  }

  /*
   * WS-Enumerate
   *
   * enumerate: List resources
   *
   * It is highly recommended to do an optimized enumeration by
   * setting the client options
   *   options.flags = Openwsman::FLAG_ENUMERATION_OPTIMIZATION
   *   options.max_elements = 999
   * to get the enumeration result as part of the http request.
   *
   * Otherwise separate pull requests are needed resulting in extra
   * round-trips (client -> wsman -> cimom & back), dramatically
   * affecting performance.
   *
   * call-seq:
   *   result = client.enumerate(options, filter, uri)
   *
   */
  WsXmlDocH enumerate( client_opt_t *options , filter_t *filter, char *resource_uri) {
    return wsmc_action_enumerate( $self, resource_uri, options, filter);
  }

  /*
   * WS-Transport
   *
   * pull: Get resources from enumeration context
   *
   * call-seq:
   *   result = client.pull(options, filter, uri, context)
   *
   */
  WsXmlDocH pull( client_opt_t *options , filter_t *filter, char *resource_uri, char *enum_ctx) {
    return wsmc_action_pull( $self, resource_uri, options, filter, enum_ctx);
  }

  /*
   * WS-Create
   *
   * create: Create a resource
   *
   * call-seq:
   *   result = client.get(options, uri, xml, xml.size, "utf-8")
   *
   */
  WsXmlDocH create( client_opt_t *options, char *resource_uri, char *data, size_t size, char *encoding = "utf-8") {
    return wsmc_action_create_fromtext( $self, resource_uri, options, data, size, encoding);
  }

  /*
   * WS-Transport
   *
   * put: Change a resource
   *
   * call-seq:
   *   result = client.get(options, uri, xml, xml.size, "utf-8")
   *
   */
  WsXmlDocH put( client_opt_t *options , char *resource_uri,  char *data, size_t size, char *encoding = "utf-8") {
    return wsmc_action_put_fromtext( $self, resource_uri, options, data, size, encoding);
  }

  /*
   * WS-Release
   *
   * release: Release enumeration context
   *
   * call-seq:
   *   result = client.release(options, uri, context)
   *
   */
  WsXmlDocH release( client_opt_t *options , char *resource_uri, char *enum_ctx) {
    return wsmc_action_release( $self, resource_uri, options, enum_ctx);
  }

  /*
   * WS-Transport
   *
   * get: Get a resource
   *
   * call-seq:
   *   result = client.get(options, uri)
   *
   */
  WsXmlDocH get( client_opt_t *options , char *resource_uri) {
    return wsmc_action_get( $self, resource_uri, options);
  }

  /*
   * WS-Transport
   *
   * delete: Delete a resource
   *
   * call-seq:
   *   result = client.delete(options, uri)
   *
   */
  WsXmlDocH delete( client_opt_t *options , char *resource_uri) {
    return wsmc_action_delete( $self, resource_uri, options);
  }

  /*
   * WS-Invoke
   *
   * invoke: Invoke a resource function
   *
   * call-seq:
   *   result = client.invoke(options, uri, "method-name")
   *   result = client.invoke(options, uri, "method-name", xml_doc)
   *
   */
  WsXmlDocH invoke( client_opt_t *options , char *resource_uri, char *method, WsXmlDocH data = NULL) {
    return wsmc_action_invoke( $self, resource_uri, options, method, data);
  }

  /*
   * WS-Eventing
   *
   * subscribe: Subscribe a listener to events
   *
   * call-seq:
   *   result = client.subscribe(options, filter, uri)
   *
   */
  WsXmlDocH subscribe(client_opt_t *options , filter_t *filter, char *resource_uri) {
    return wsmc_action_subscribe($self,  resource_uri, options, filter);
  }

  /*
   * WS-Eventing
   *
   * unsubscribe: Remove a listener from events
   *
   * call-seq:
   *   result = client.unsubscribe(options, filter, uri, identifier)
   *
   */
  WsXmlDocH unsubscribe(client_opt_t *options , filter_t *filter, char *resource_uri, char *identifier) {
    return wsmc_action_unsubscribe($self, resource_uri, options, identifier);
  }

  /*
   * WS-Eventing
   *
   * renew: Renew a subscription
   *
   * call-seq:
   *   result = client.renew(options, uri, identifier)
   *
   */
  WsXmlDocH renew(client_opt_t *options , char *resource_uri, char *identifier) {
    return wsmc_action_renew($self, resource_uri, options, identifier);
  }

  /*
   * Get a string representation of the last fault
   *
   * call-seq:
   *   client.fault_string
   *
   */
  char *fault_string() {
    return wsmc_get_fault_string($self);
  }
  
  /*
   * Get a numeric representation of the last fault
   *
   * call-seq:
   *   client.last_error
   *
   */
   int last_error() {
     return wsmc_get_last_error($self);
   }
}
