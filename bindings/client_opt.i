/*
 *
 * ClientOptions
 *
 * client option declarations for openwsman swig bindings
 *
 */

%rename(ClientOptions) client_opt_t;
%nodefault client_opt_t;
typedef struct {} client_opt_t;


/*
 * ClientOptions control the behaviour of the Client connection
 *
 * The primary use of ClientOptions in normal operations is adding
 * selectors - key/value pairs added to the request URL.
 *
 * For WS-CIM operations, selectors define the key attributes for
 * the selected CIM class to address a specific instance of the class.
 *
 */

%extend client_opt_t {
  client_opt_t() {
    client_opt_t *options = wsmc_options_init();
#if defined(SWIGJAVA)
    if (options) {
	    options->selectors = hash_create3(HASHCOUNT_T_MAX, 0, 0);
	    options->properties = hash_create3(HASHCOUNT_T_MAX, 0, 0);
    }
#endif
    return options;
  }

  ~client_opt_t() {
    wsmc_options_destroy( $self );
  }

  /*
   * Request to dump all operations to the dumpfile
   *
   * Used for debugging on the wire-level
   *
   */
  void set_dump_request(void) {
    wsmc_set_action_option($self, FLAG_DUMP_REQUEST );
  }

  /*
   * Reset dump all operations to the dumpfile
   *
   * Used for debugging on the wire-level
   *
   */
  void clear_dump_request(void) {
    wsmc_clear_action_option($self, FLAG_DUMP_REQUEST );
  }

  /*
   * set option flag(s)
   *
   */
#if defined(SWIGRUBY)
  %rename( "flags=" ) set_flags(int flags);
#endif
  void set_flags(int flags) {
    wsmc_set_action_option($self, flags);
  }

  /*
   * clear option flag(s)
   *
   */
  void clear_flags(int flags) {
    wsmc_clear_action_option($self, flags);
  }

  /*
   * Limit size of result document
   *
   */
#if defined(SWIGRUBY)
  %rename( "max_envelope_size=" ) set_max_envelope_size(unsigned long size);
#endif
  void set_max_envelope_size(unsigned long size) {
    $self->max_envelope_size = size;
  }

#if defined(SWIGRUBY)
  %rename( "max_envelope_size" ) get_max_envelope_size();
#endif
  unsigned long get_max_envelope_size() {
    return $self->max_envelope_size;
  }
   
  /*
   * Limit number of elements returned by enumeration
   *
   */
#if defined(SWIGRUBY)
  %rename( "max_elements=" ) set_max_elements(int elements);
#endif
  void set_max_elements(int elements) {
    $self->max_elements = elements;
  }

#if defined(SWIGRUBY)
  %rename( "max_elements" ) get_max_elements();
#endif
  int get_max_elements() {
    return $self->max_elements;
  }

  /*
   * Operation timeout
   * See Openwsman::Transport.timeout for transport timeout
   *
   */
#if defined(SWIGRUBY)
  %rename( "timeout=" ) set_timeout(unsigned long timeout);
#endif
  void set_timeout(unsigned long timeout) {
    $self->timeout = timeout;
  }

#if defined(SWIGRUBY)
  %rename( "timeout" ) get_timeout();
#endif
  unsigned long get_timeout() {
    return $self->timeout;
  }
   
  /*
   * Fragment
   * (Supported Dialects: XPATH)
   *
   */
#if defined(SWIGRUBY)
  %rename( "fragment=" ) set_fragment(char *fragment);
#endif
  void set_fragment(char *fragment) {
    wsmc_set_fragment(fragment, $self);
  }

#if defined(SWIGRUBY)
  %rename( "fragment" ) get_fragment();
#endif
  const char *get_fragment() {
    return $self->fragment;
  }
   
  /*
   * CIM Namespace
   * (default is root/cimv2)
   *
   */
#if defined(SWIGRUBY)
  %rename( "cim_namespace=" ) set_cim_namespace(char *cim_namespace);
#endif
  void set_cim_namespace(char *cim_namespace) {
    wsmc_set_cim_ns(cim_namespace, $self);
  }

#if defined(SWIGRUBY)
  %rename( "cim_namespace" ) get_cim_namespace();
#endif
  const char *get_cim_namespace() {
    return $self->cim_ns;
  }
   
  /*
   * Reference
   * (XML string)
   *
   */
#if defined(SWIGRUBY)
  %rename( "reference=" ) set_reference(const char *reference);
#endif
  void set_reference(const char *reference) {
    wsmc_set_reference(reference, $self);
  }

#if defined(SWIGRUBY)
  %rename( "reference" ) get_reference();
#endif
  const char *get_reference() {
    return $self->reference;
  }
   
  /*
   * Add a selector as key/value pair
   *
   */
#if defined(SWIGRUBY)
  void add_selector(VALUE k, VALUE v)
  {
    const char *key = as_string(k);
    const char *value = as_string(v);
#else
  void add_selector(const char *key, const char *value)
  {
#endif
#if defined(SWIGJAVA)
    key = strdup(key);
    value = strdup(value);
#endif
    wsmc_add_selector($self, key, value);
  }

#if defined(SWIGRUBY)
  /*
   * Set selectors from Hash
   */
  %rename( "selectors=" ) set_selectors(VALUE hash);
  void set_selectors(VALUE hash)
  {
    $self->selectors = value2hash(NULL, hash);
  }
#endif

  /*
   * Add a property as key/value pair
   *
   */
#if defined(SWIGRUBY)
  void add_property(VALUE k, VALUE v)
  {
    const char *key = as_string(k);
    const char *value = as_string(v);
#else
  void add_property(const char *key, const char *value)
  {
#endif
#if defined(SWIGJAVA)
    key = strdup(key);
    value = strdup(value);
#endif
    wsmc_add_property($self, key, value);
  }
  
#if defined(SWIGRUBY)
  /*
   * Set properties from Hash
   */
  %rename( "properties=" ) set_properties(VALUE hash);
  void set_properties(VALUE hash)
  {
    $self->properties = value2hash(NULL, hash);
  }
#endif

  /*
   * Set delivery uri
   */
#if defined(SWIGRUBY)
  %rename( "delivery_uri=" ) set_delivery_uri(const char *delivery_uri);
#endif
  void set_delivery_uri( const char *delivery_uri ) {
    wsmc_set_delivery_uri(delivery_uri, $self);
  }

  /*
   * Get delivery uri
   */
  const char *delivery_uri() {
    return $self->delivery_uri;
  }

  /*
   * Set subscription expiry timeout (in seconds)
   */
#if defined(SWIGRUBY)
  %rename( "sub_expiry=" ) set_sub_expiry(unsigned int event_subscription_expire);
#endif
  void set_sub_expiry(unsigned int event_subscription_expire) {
	wsmc_set_sub_expiry(event_subscription_expire, $self);
  }

  int sub_expiry() {
    return $self->expires;
  }

  /*
   * Set subscription heartbeat interval (in seconds)
   */
#if defined(SWIGRUBY)
  %rename("heartbeat_interval=") set_heartbeat_interval(unsigned int heartbeat_interval);
#endif
  void set_heartbeat_interval(unsigned int heartbeat_interval) {
	wsmc_set_heartbeat_interval(heartbeat_interval, $self);
  }

  int heartbeat_interval() {
    return $self->heartbeat_interval;
  }

  /*
   * Set subscription delivery mode (push, pushwithack,events,pull)
   */
#if defined(SWIGRUBY)
  %rename( "delivery_mode=" ) set_delivery_mode(unsigned int delivery_mode);
#endif
  void set_delivery_mode(unsigned int delivery_mode) {
    if (delivery_mode > WSMAN_DELIVERY_PULL)
      SWIG_exception( SWIG_ValueError, "Bad delivery mode" );
	
    wsmc_set_delivery_mode(delivery_mode, $self);
#if defined(SWIGPYTHON) || defined(SWIGPERL) || defined(SWIGJAVA)
    fail:
    return;
#endif
  }

  int delivery_mode() {
    return $self->delivery_mode;
  }

  /*
   * Set subscription delivery security mode (lots)
   */
#if defined(SWIGRUBY)
  %rename( "delivery_security_mode=" ) set_delivery_sec_mode(unsigned int delivery_mode);
#endif
  void set_delivery_security_mode(unsigned int delivery_sec_mode) {
    if (delivery_sec_mode > WSMAN_DELIVERY_SEC_HTTP_SPNEGO_KERBEROS)
      SWIG_exception( SWIG_ValueError, "Bad delivery security mode" );
    wsmc_set_delivery_security_mode(delivery_sec_mode, $self);
#if defined(SWIGPYTHON) || defined(SWIGPERL) || defined(SWIGJAVA)
    fail:
    return;
#endif
  }
  
  int delivery_sec_mode() {
    return $self->delivery_sec_mode;
  }

}

