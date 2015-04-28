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
 * Document-class: ClientOptions
 *
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
   * See also: clear_dump_request
   *
   * call-seq:
   *   options.set_dump_request
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
   * See also: set_dump_request
   *
   * call-seq:
   *   options.clear_dump_request
   *
   */
  void clear_dump_request(void) {
    wsmc_clear_action_option($self, FLAG_DUMP_REQUEST );
  }

#if defined(SWIGRUBY)
  %rename( "flags=" ) set_flags(int flags);
#endif
  /*
   * set option flag(s)
   * 
   * adds new flag(s) to options
   *
   * call-seq:
   *   options.flags = Openwsman::FLAG_ENUMERATION_OPTIMIZATION
   *
   */
  void set_flags(int flags) {
    wsmc_set_action_option($self, flags);
  }

#if defined(SWIGRUBY)
  %rename( "flags" ) get_flags();
#endif
  /*
   * get option flag(s)
   * 
   * return current flags bitmask
   *
   * call-seq:
   *   optins.flags -> Integer
   *
   */
  unsigned int get_flags() {
    return wsmc_get_action_option($self);
  }

  /*
   * clear option flag(s)
   * 
   * clears specific flag(s) from options
   *
   * call-seq:
   *   options.clear_flags Openwsman::FLAG_ENUMERATION_OPTIMIZATION
   *
   */
  void clear_flags(int flags) {
    wsmc_clear_action_option($self, flags);
  }

  /*
   * reset option flags
   * 
   * sets option flags bitmask to FLAG_NONE
   *
   * call-seq:
   *   options.reset_flags
   *
   */
  void reset_flags() {
    wsmc_clear_action_option($self, ~FLAG_NONE);
  }

#if defined(SWIGRUBY)
  %rename( "max_envelope_size=" ) set_max_envelope_size(unsigned long size);
#endif
  /*
   * Limit size of result document
   *
   * call-seq:
   *   options.max_envelope_size = 10240
   *
   */
  void set_max_envelope_size(unsigned long size) {
    $self->max_envelope_size = size;
  }

#if defined(SWIGRUBY)
  %rename( "max_envelope_size" ) get_max_envelope_size();
#endif
  /*
   * Return size limit of result document
   *
   * call-seq:
   *   options.max_envelope_size -> Integer
   *
   */
  unsigned long get_max_envelope_size() {
    return $self->max_envelope_size;
  }
   
#if defined(SWIGRUBY)
  %rename( "max_elements=" ) set_max_elements(int elements);
#endif
  /*
   * Limit number of elements returned by enumeration
   *
   * call-seq:
   *   options.max_elements = 42
   *
   */
  void set_max_elements(int elements) {
    $self->max_elements = elements;
  }

#if defined(SWIGRUBY)
  %rename( "max_elements" ) get_max_elements();
#endif
  /*
   * Return enumeration elements limit
   *
   * call-seq:
   *   options.max_elements -> Integer
   *
   */
  int get_max_elements() {
    return $self->max_elements;
  }

#if defined(SWIGRUBY)
  %rename( "timeout=" ) set_timeout(unsigned long timeout);
#endif
  /*
   * Operation timeout in milliseconds
   * See Openwsman::Transport.timeout for transport timeout
   *
   * call-seq:
   *   options.timeout = 60*1000 # 60 seconds
   *
   */
  void set_timeout(unsigned long timeout) {
    $self->timeout = timeout;
  }

#if defined(SWIGRUBY)
  %rename( "timeout" ) get_timeout();
#endif
  /*
   * Return operation timeout in milliseconds
   * See Openwsman::Transport.timeout for transport timeout
   *
   * call-seq:
   *   options.timeout -> Integer
   *
   */
  unsigned long get_timeout() {
    return $self->timeout;
  }
   
#if defined(SWIGRUBY)
  %rename( "fragment=" ) set_fragment(char *fragment);
#endif
  /*
   * Set fragment filter
   * See DSP0226, section 7.7.
   * (Supported Dialects: XPATH)
   *
   * call-seq:
   *   options.fragment = "xpath/expression"
   *
   */
  void set_fragment(char *fragment) {
    wsmc_set_fragment(fragment, $self);
  }

#if defined(SWIGRUBY)
  %rename( "fragment" ) get_fragment();
#endif
  /*
   * Get fragment filter
   * See DSP0226, section 7.7.
   *
   * call-seq:
   *   options.fragment -> String
   *
   */
  const char *get_fragment() {
    return $self->fragment;
  }
   
#if defined(SWIGRUBY)
  %rename( "cim_namespace=" ) set_cim_namespace(char *cim_namespace);
#endif
  /*
   * Set CIM Namespace for Openwsman
   * (default is root/cimv2)
   * Note:
   * Microsoft WinRM set the resource namespace by attaching it
   * to the resource URI
   *
   * See also: Openwsman.epr_prefix_for
   *
   * call-seq:
   *   options.cim_namespace = "root/interop"
   *
   */
  void set_cim_namespace(char *cim_namespace) {
    wsmc_set_cim_ns(cim_namespace, $self);
  }

#if defined(SWIGRUBY)
  %rename( "cim_namespace" ) get_cim_namespace();
#endif
  /*
   * Get CIM Namespace for Openwsman
   * Note:
   * Microsoft WinRM set the resource namespace by attaching it
   * to the resource URI
   *
   * See also: Openwsman.epr_prefix_for
   *
   * call-seq:
   *   options.cim_namespace -> String
   *
   */
  const char *get_cim_namespace() {
    return $self->cim_ns;
  }

#if defined(SWIGRUBY)
  %rename( "reference=" ) set_reference(const char *reference);
#endif
  /*
   * Set WS-Addressing reference properties
   * Argument must the string representation of a valid XML document
   *
   * call-seq:
   *   options.reference = "<xml ...>"
   *
   */
  void set_reference(const char *reference) {
    wsmc_set_reference(reference, $self);
  }

#if defined(SWIGRUBY)
  %rename( "reference" ) get_reference();
#endif
  /*
   * Get WS-Addressing reference properties
   * Returns the string representation of a valid XML document
   *
   * call-seq:
   *   options.reference -> String
   *
   */
  const char *get_reference() {
    return $self->reference;
  }
   
#if defined(SWIGRUBY)
  /*
   * Add an option (for OptionSet) as key/value pair
   *
   * NOTE:
   * the value must be properly escaped (replace & with &amp;, etc.)
   * in Ruby use CGI::escapeHTML()
   *
   * call-seq:
   *   options.add_option "Name", "Value"
   *
   */
  void add_option(VALUE k, VALUE v)
  {
    const char *key = as_string(k);
    const char *value = as_string(v);
#else
  void add_option(const char *key, const char *value)
  {
#endif
    wsmc_add_option($self, key, value);
  }

#if defined(SWIGRUBY)
  %rename( "options=" ) set_options(VALUE hash);
  /*
   * Set options (for OptionSet) from Hash
   *
   * NOTE:
   * the values must be properly escaped (replace & with &amp;, etc.)
   * in Ruby use CGI::escapeHTML()
   *
   * call-seq:
   *   options.options = { "Name" => "Value", ... }
   *
   */
  void set_options(VALUE hash)
  {
    $self->options = value2hash(NULL, hash, 0);
  }

  %rename( "options" ) get_options(void);
  /*
   * Get options (for OptionSet) as Hash
   *
   * call-seq:
   *   options.options -> Hash
   *
   */
  VALUE get_options(void)
  {
    return hash2value($self->options);
  }
#endif

#if defined(SWIGRUBY)
  /*
   * Add a selector as key/value pair
   *
   * NOTE:
   * the string value must be properly escaped (replace & with &amp;, etc.)
   * in Ruby use CGI::escapeHTML()
   *
   * call-seq:
   *   options.add_selector "Key", "Value"
   *   options.add_selector "Key", end_point_reference
   *
   */
  void add_selector(VALUE k, VALUE v)
  {
    const char *key = as_string(k);
    KLASS_DECL(SwigClassEndPointReference,SWIGTYPE_p_epr_t);
    if (CLASS_OF(v) == KLASS_OF(SwigClassEndPointReference)) {
      epr_t *epr;
      SWIG_ConvertPtr(v, (void **)&epr, SWIGTYPE_p_epr_t, 0);
      wsmc_add_selector_epr($self, key, epr);
    }
    else {
      const char *value = as_string(v);
      wsmc_add_selector($self, key, value);
    }
  }
#else
  void add_selector(const char *key, const char *value)
  {
    wsmc_add_selector($self, key, value);
  }
#endif

#if defined(SWIGRUBY)
  %rename( "selectors" ) get_selectors(void);
  /*
   * Get selectors as Hash
   *
   * call-seq:
   *   options.selectors -> Hash
   *
   */
  VALUE get_selectors(void)
  {
    return kv_list_to_hash($self->selectors);
  }
#endif

#if defined(SWIGRUBY)
  /*
   * Add a property as key/value pair
   * * Input parameters to 'invoke'd methods are represented as ClientOption properties
   * * Key is evaluated as String
   * * Value is evaluated as String or EndPointReference
   *
   * call-seq:
   *   options.add_property "Key", "Value"
   *   options.add_property "Key", EndPointReference.new(...)
   *
   */
  void add_property(VALUE k, VALUE v)
  {
    const char *key = as_string(k);
    KLASS_DECL(SwigClassEndPointReference,SWIGTYPE_p_epr_t);

    if (CLASS_OF(v) == KLASS_OF(SwigClassEndPointReference)) {
      const epr_t *epr;
      SWIG_ConvertPtr(v, (void **)&epr, SWIGTYPE_p_epr_t, 0);
      wsmc_add_property_epr($self, key, epr);
    }
    else {
      const char *value = as_string(v);
      wsmc_add_property($self, key, value);
    }
  }
#else
  /*
   * Add a string property as key/value pair
   * * Input parameters to 'invoke'd methods are represented as ClientOption properties
   *
   * call-seq:
   *   options.add_property( "Key", "Value" )
   *
   */
  void add_property(const char *key, const char *value)
  {
    wsmc_add_property($self, key, value);
  }

  /*
   * Add an EndPointReference property as key/value pair
   *   Input parameters to 'invoke'd methods are represented as ClientOption properties
   *
   * call-seq:
   *   options.add_property( String, EndPointReference )
   *
   */
  void add_property(const char *key, const epr_t *epr)
  {
    wsmc_add_property_epr($self, key, epr);
  }
#endif
  
#if defined(SWIGRUBY)
  %rename( "properties" ) get_properties(void);
  /*
   * Get properties as Hash
   * * Input parameters to 'invoke'd methods are represented as ClientOption properties
   *
   * call-seq:
   *   options.properties -> Hash
   *
   */
  VALUE get_properties(void)
  {
    return kv_list_to_hash($self->properties);
  }
#endif

#if defined(SWIGRUBY)
  %rename( "delivery_uri=" ) set_delivery_uri(const char *delivery_uri);
#endif
  /*
   * Set delivery uri
   *
   * call-seq:
   *   options.delivery_uri = "http://..."
   *
   */
  void set_delivery_uri( const char *delivery_uri ) {
    wsmc_set_delivery_uri(delivery_uri, $self);
  }

  /*
   * Get delivery uri
   *
   * call-seq:
   *   options.delivery_uri -> String
   *
   */
  const char *delivery_uri() {
    return $self->delivery_uri;
  }

#if defined(SWIGRUBY)
  %rename( "sub_expiry=" ) set_sub_expiry(unsigned int event_subscription_expire);
#endif
  /*
   * Set subscription expiry timeout (in seconds)
   *
   * call-seq:
   *   options.sub_expiry = 600 # 10 mins
   *
   */
  void set_sub_expiry(unsigned int event_subscription_expire) {
	wsmc_set_sub_expiry(event_subscription_expire, $self);
  }

  /*
   * Get subscription expiry timeout (in seconds)
   *
   * call-seq:
   *   options.sub_expiry -> Integer
   *
   */
  int sub_expiry() {
    return $self->expires;
  }

#if defined(SWIGRUBY)
  %rename("heartbeat_interval=") set_heartbeat_interval(unsigned int heartbeat_interval);
#endif
  /*
   * Set subscription heartbeat interval (in seconds)
   *
   * call-seq:
   *   options.heartbeat_interval = 60 # every minute
   *
   */
  void set_heartbeat_interval(unsigned int heartbeat_interval) {
	wsmc_set_heartbeat_interval(heartbeat_interval, $self);
  }

  /*
   * Get subscription heartbeat interval (in seconds)
   *
   * call-seq:
   *   options.heartbeat_interval -> Integer
   *
   */
  int heartbeat_interval() {
    return $self->heartbeat_interval;
  }

#if defined(SWIGRUBY)
  %rename( "delivery_mode=" ) set_delivery_mode(unsigned int delivery_mode);
#endif
  /*
   * Set subscription delivery mode (push, pushwithack,events,pull)
   *
   * call-seq:
   *   options.delivery_mode = Openwsman::WSMAN_DELIVERY_PUSH
   *
   */
  void set_delivery_mode(unsigned int delivery_mode) {
    if (delivery_mode > WSMAN_DELIVERY_PULL)
      SWIG_exception( SWIG_ValueError, "Bad delivery mode" );
	
    wsmc_set_delivery_mode(delivery_mode, $self);
#if defined(SWIGPYTHON) || defined(SWIGPERL) || defined(SWIGJAVA)
    fail:
    return;
#endif
  }

  /*
   * Get subscription delivery mode (push, pushwithack,events,pull)
   *
   * call-seq:
   *   options.delivery_mode -> Integer
   *
   */
  int delivery_mode() {
    return $self->delivery_mode;
  }

#if defined(SWIGRUBY)
  %rename( "delivery_security_mode=" ) set_delivery_security_mode(unsigned int delivery_mode);
#endif
  /*
   * Set subscription delivery security mode
   *
   * (auto, http basic, http digest, https basic, https digest,
   * https mutual, https mutual basic, https mutual digest,
   * http spnego kerberos, https spnego kerberos,
   * https mutual spnego kerberos)
   *
   * call-seq:
   *   options.delivery_security_mode = Openwsman::WSMAN_DELIVERY_SEC_HTTPS_BASIC
   *
   */
  void set_delivery_security_mode(unsigned int delivery_sec_mode) {
    if (delivery_sec_mode > WSMAN_DELIVERY_SEC_HTTP_SPNEGO_KERBEROS)
      SWIG_exception( SWIG_ValueError, "Bad delivery security mode" );
    wsmc_set_delivery_security_mode(delivery_sec_mode, $self);
#if defined(SWIGPYTHON) || defined(SWIGPERL) || defined(SWIGJAVA)
    fail:
    return;
#endif
  }
  
  /*
   * Get subscription delivery security mode
   *
   * call-seq:
   *   options.delivery_security_mode -> Integer
   *
   */
  int delivery_sec_mode() {
    return $self->delivery_sec_mode;
  }

}
