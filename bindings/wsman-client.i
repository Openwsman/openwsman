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
 * Document-class: Client
 *
 * Instances of Client represent a connection to a client used for
 * sending WS-Management operation requests.
 *
 */
 
%extend _WsManClient {
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
  _WsManClient( const char *uri ) {
    struct _WsManClient *client = wsmc_create_from_uri( uri );
    if (client == NULL)
      SWIG_exception( SWIG_ValueError, "Can't create Openwsman::Client from given URI" );
#if defined(SWIGPYTHON) || defined(SWIGPERL) || defined(SWIGJAVA)
    fail:
#endif
    return client;
  }

  /*
   * :nodoc:
   */
  _WsManClient(const char *hostname,
              const int port, const char *path,
              const char *scheme,
              const char *username,
              const char *password) {
    struct _WsManClient *client = wsmc_create( hostname, port, path, scheme, username, password );
    if (client == NULL)
      SWIG_exception( SWIG_ValueError, "Can't create Openwsman::Client from given values" );
#if defined(SWIGPYTHON) || defined(SWIGPERL) || defined(SWIGJAVA)
    fail:
#endif
    return client;
  }

  /* destructor */
  ~_WsManClient() {
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

  %newobject scheme;
  /*
   * String representation of the transport scheme
   *
   * call-seq:
   *   client.scheme -> String
   *
   */
  char *scheme() {
    return wsmc_get_scheme( $self );
  }

  %newobject host;
  /*
   * The host part of the client URL
   *
   */
  char *host() {
    return wsmc_get_hostname( $self );
  }

  /*
   * The TCP port used in the connection
   *
   */
  int port() {
    return wsmc_get_port( $self );
  }

  %newobject path;
  /*
   * The path of the clien URL
   *
   */
  char *path() {
    return wsmc_get_path( $self );
  }

  %newobject user;
  /*
   * The user name used for authentication
   *
   */
  char *user() {
    return wsmc_get_user( $self );
  }

  %newobject password;
  /*
   * The password used for authentication
   *
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
   *   client.send_request(XmlDoc.new("<xml ...>...</xml>")) -> Integer
   *
   */
  int send_request(WsXmlDocH request) {
    return wsman_send_request($self, request);
  }

  /*
   * Build envelope from response
   *
   * call-seq:
   *   client.build_envelope_from_response() -> XmlDoc
   *
   */
  WsXmlDocH build_envelope_from_response() {
    return wsmc_build_envelope_from_response($self);
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
   *   client.identify(options) -> XmlDoc
   *
   */
  WsXmlDocH identify( client_opt_t *options ) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_identify_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_identify_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_identify( $self, options );
#endif
  }
  
  /*
   * WS-Get
   *
   * get_from_epr: Get a resource via an endpoint reference
   *
   * call-seq:
   *   client.get_from_epr(options, end_point_reference) -> XmlDoc
   *
   */
  WsXmlDocH get_from_epr( client_opt_t *options , epr_t *epr) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.epr = epr;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_get_from_epr_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_get_from_epr_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_get_from_epr( $self, epr, options);
#endif
  }

  /*
   * WS-Delete
   *
   * delete_from_epr: Remove a resource via an endpoint reference
   *
   * call-seq:
   *   client.delete_from_epr(options, end_point_reference) -> XmlDoc
   *
   */
  WsXmlDocH delete_from_epr( client_opt_t *options , epr_t *epr) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.epr = epr;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_delete_from_epr_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_delete_from_epr_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_delete_from_epr( $self, epr, options);
#endif
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
   *   client.enumerate(options, filter, uri) -> XmlDoc
   *
   */
  WsXmlDocH enumerate( client_opt_t *options , filter_t *filter, char *resource_uri) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.filter = filter;
    args.resource_uri = resource_uri;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_enumerate_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_enumerate_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_enumerate( $self, resource_uri, options, filter);
#endif
  }

  /*
   * WS-Transport
   *
   * pull: Get resources from enumeration context
   *
   * call-seq:
   *   client.pull(options, filter, uri, context) -> XmlDoc
   *
   */
  WsXmlDocH pull( client_opt_t *options , filter_t *filter, const char *resource_uri, const char *context) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.filter = filter;
    args.resource_uri = resource_uri;
    args.context = context;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_pull_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_pull_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_pull( $self, resource_uri, options, filter, context);
#endif
  }

  /*
   * WS-Create
   *
   * create: Create a resource
   *
   * call-seq:
   *   client.create(options, uri, xml, xml.size, "utf-8") -> XmlDoc
   *
   */
  WsXmlDocH create( client_opt_t *options, const char *resource_uri, const char *data, size_t size, const char *encoding = "utf-8") {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.resource_uri = resource_uri;
    args.data = data;
    args.size = size;
    args.encoding = encoding;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_create_fromtext_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_create_fromtext_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_create_fromtext( $self, resource_uri, options, data, size, encoding);
#endif
  }

  /*
   * WS-Transport
   *
   * put: Change a resource
   *
   * call-seq:
   *   client.put(options, uri, xml, xml.size, "utf-8") -> XmlDoc
   *
   */
  WsXmlDocH put( client_opt_t *options, const char *resource_uri, const char *data, size_t size, const char *encoding = "utf-8") {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.resource_uri = resource_uri;
    args.data = data;
    args.size = size;
    args.encoding = encoding;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_put_fromtext_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_put_fromtext_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_put_fromtext( $self, resource_uri, options, data, size, encoding);
#endif
  }

  /*
   * WS-Release
   *
   * release: Release enumeration context
   *
   * call-seq:
   *   client.release(options, uri, context) -> XmlDoc
   *
   */
  WsXmlDocH release( client_opt_t *options, const char *resource_uri, const char *context) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.resource_uri = resource_uri;
    args.context = context;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_release_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_release_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_release( $self, resource_uri, options, context);
#endif
  }

  /*
   * WS-Transport
   *
   * get: Get a resource
   *
   * call-seq:
   *   client.get(options, uri) -> XmlDoc
   *
   */
  WsXmlDocH get( client_opt_t *options, const char *resource_uri) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.resource_uri = resource_uri;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_get_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_get_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_get( $self, resource_uri, options);
#endif
  }

  /*
   * WS-Transport
   *
   * delete: Delete a resource
   *
   * call-seq:
   *   client.delete(options, uri) -> XmlDoc
   *
   */
  WsXmlDocH delete( client_opt_t *options, const char *resource_uri) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.resource_uri = resource_uri;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_delete_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_delete_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_delete( $self, resource_uri, options);
#endif
  }

  /*
   * WS-Invoke
   *
   * invoke: Invoke a resource function
   *
   * call-seq:
   *   client.invoke(options, uri, "method-name") -> XmlDoc
   *   client.invoke(options, uri, "method-name", xml_doc) -> XmlDoc
   *
   */
  WsXmlDocH invoke( client_opt_t *options, const char *resource_uri, const char *method, WsXmlDocH data = NULL) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.resource_uri = resource_uri;
    args.method = method;
    args.method_args = data;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_invoke_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_invoke_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_invoke( $self, resource_uri, options, method, data);
#endif
  }

  /*
   * WS-Eventing
   *
   * subscribe: Subscribe a listener to events
   *
   * call-seq:
   *   client.subscribe(options, filter, uri) -> XmlDoc
   *
   */
  WsXmlDocH subscribe(client_opt_t *options, filter_t *filter, const char *resource_uri) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.filter = filter;
    args.resource_uri = resource_uri;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_subscribe_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_subscribe_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_subscribe($self,  resource_uri, options, filter);
#endif
  }

  /*
   * WS-Eventing
   *
   * unsubscribe: Remove a listener from events
   *
   * call-seq:
   *   client.unsubscribe(options, filter, uri, identifier) -> XmlDoc
   *
   */
  WsXmlDocH unsubscribe(client_opt_t *options, filter_t *filter, const char *resource_uri, const char *identifier) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.filter = filter;
    args.resource_uri = resource_uri;
    args.identifier = identifier;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_unsubscribe_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_unsubscribe_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_unsubscribe($self, resource_uri, options, identifier);
#endif
  }

  /*
   * WS-Eventing
   *
   * renew: Renew a subscription
   *
   * call-seq:
   *   client.renew(options, uri, identifier) -> XmlDoc
   *
   */
  WsXmlDocH renew(client_opt_t *options , char *resource_uri, char *identifier) {
#if RUBY_VERSION > 18 /* YARV */
    wsmc_action_args_t args;
    args.client = $self;
    args.options = options;
    args.resource_uri = resource_uri;
    args.identifier = identifier;
#if RUBY_VERSION > 20 /* New threading model */
    return (WsXmlDocH)rb_thread_call_without_gvl((void * (*)(void *))ruby_renew_thread, &args, RUBY_UBF_IO, 0);
#else
    return (WsXmlDocH)rb_thread_blocking_region((rb_blocking_function_t*)ruby_renew_thread, &args, RUBY_UBF_IO, 0);
#endif
#else
    return wsmc_action_renew($self, resource_uri, options, identifier);
#endif
  }

  /*
   * Get a string representation of the last fault
   *
   * call-seq:
   *   client.fault_string -> String
   *
   */
  char *fault_string() {
    return wsmc_get_fault_string($self);
  }
  
  /*
   * Get a numeric representation of the last fault
   *
   * call-seq:
   *   client.last_error -> Integer
   *
   */
   int last_error() {
     return wsmc_get_last_error($self);
   }
}
