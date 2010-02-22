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
    return wsmc_options_init();
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
   * set option flag(s)
   *
   */
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
   * Add a selector as key/value pair
   *
   */
#if defined(SWIGRUBY)
  void add_selector(VALUE k, VALUE v)
  {
    VALUE k_s = rb_funcall(k, rb_intern("to_s"), 0 );
    VALUE v_s = rb_funcall(v, rb_intern("to_s"), 0 );
    const char *key = StringValuePtr(k_s);
    const char *value = StringValuePtr(v_s);
#else
  void add_selector(const char *key, const char *value)
  {
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
    VALUE k_s = rb_funcall(k, rb_intern("to_s"), 0 );
    VALUE v_s = rb_funcall(v, rb_intern("to_s"), 0 );
    const char *key = StringValuePtr(k_s);
    const char *value = StringValuePtr(v_s);
#else
  void add_property(const char *key, const char *value)
  {
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
  }
  
  int delivery_sec_mode() {
    return $self->delivery_sec_mode;
  }

}

