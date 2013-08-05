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
	    options->options = hash_create3(HASHCOUNT_T_MAX, 0, 0);
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
   * See also: clear_dump_request
   *
   * call-seq:
   *   options.set_dump_request
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
   */
  void clear_dump_request(void) {
    wsmc_clear_action_option($self, FLAG_DUMP_REQUEST );
  }

  /*
   * set option flag(s)
   * 
   * adds new flag(s) to options
   *
   * call-seq:
   *   options.flags = Openwsman::FLAG_ENUMERATION_OPTIMIZATION
   */
#if defined(SWIGRUBY)
  %rename( "flags=" ) set_flags(int flags);
#endif
  void set_flags(int flags) {
    wsmc_set_action_option($self, flags);
  }

  /*
   * get option flag(s)
   * 
   * return current flags bitmask
   *
   * call-seq:
   *   optins.flags -> Integer
   */
#if defined(SWIGRUBY)
  %rename( "flags" ) get_flags();
#endif
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
   */
  void reset_flags() {
    wsmc_clear_action_option($self, ~FLAG_NONE);
  }

  /*
   * Limit size of result document
   *
   * call-seq:
   *   options.max_envelope_size = 10240
   */
#if defined(SWIGRUBY)
  %rename( "max_envelope_size=" ) set_max_envelope_size(unsigned long size);
#endif
  void set_max_envelope_size(unsigned long size) {
    $self->max_envelope_size = size;
  }

  /*
   * Return size limit of result document
   *
   * call-seq:
   *   options.max_envelope_size -> Integer
   */
#if defined(SWIGRUBY)
  %rename( "max_envelope_size" ) get_max_envelope_size();
#endif
  unsigned long get_max_envelope_size() {
    return $self->max_envelope_size;
  }
   
  /*
   * Limit number of elements returned by enumeration
   *
   * call-seq:
   *   options.max_elements = 42
   */
#if defined(SWIGRUBY)
  %rename( "max_elements=" ) set_max_elements(int elements);
#endif
  void set_max_elements(int elements) {
    $self->max_elements = elements;
  }

  /*
   * Return enumeration elements limit
   *
   * call-seq:
   *   options.max_elements -> Integer
   */
#if defined(SWIGRUBY)
  %rename( "max_elements" ) get_max_elements();
#endif
  int get_max_elements() {
    return $self->max_elements;
  }

  /*
   * Operation timeout in milliseconds
   * See Openwsman::Transport.timeout for transport timeout
   *
   * call-seq:
   *   options.timeout = 60*1000 # 60 seconds
   */
#if defined(SWIGRUBY)
  %rename( "timeout=" ) set_timeout(unsigned long timeout);
#endif
  void set_timeout(unsigned long timeout) {
    $self->timeout = timeout;
  }

  /*
   * Return operation timeout in milliseconds
   * See Openwsman::Transport.timeout for transport timeout
   *
   * call-seq:
   *   options.timeout -> Integer
   */
#if defined(SWIGRUBY)
  %rename( "timeout" ) get_timeout();
#endif
  unsigned long get_timeout() {
    return $self->timeout;
  }
   
  /*
   * Set fragment filter
   * See DSP0226, section 7.7.
   * (Supported Dialects: XPATH)
   *
   * call-seq:
   *   options.fragment = "xpath/expression"
   */
#if defined(SWIGRUBY)
  %rename( "fragment=" ) set_fragment(char *fragment);
#endif
  void set_fragment(char *fragment) {
    wsmc_set_fragment(fragment, $self);
  }

  /*
   * Get fragment filter
   * See DSP0226, section 7.7.
   *
   * call-seq:
   *   options.fragment -> String
   */
#if defined(SWIGRUBY)
  %rename( "fragment" ) get_fragment();
#endif
  const char *get_fragment() {
    return $self->fragment;
  }
   
  /*
   * Set CIM Namespace for Openwsman
   * (default is root/cimv2)
   * Note: Microsoft WinRM set the resource namespace by attaching it
   *       to the resource URI
   * See also: Openwsman.epr_prefix_for
   *
   * call-seq:
   *   options.cim_namespace = "root/interop"
   */
#if defined(SWIGRUBY)
  %rename( "cim_namespace=" ) set_cim_namespace(char *cim_namespace);
#endif
  void set_cim_namespace(char *cim_namespace) {
    wsmc_set_cim_ns(cim_namespace, $self);
  }

  /*
   * Get CIM Namespace for Openwsman
   * Note: Microsoft WinRM set the resource namespace by attaching it
   *       to the resource URI
   * See also: Openwsman.epr_prefix_for
   *
   * call-seq:
   *   options.cim_namespace -> String
   */
#if defined(SWIGRUBY)
  %rename( "cim_namespace" ) get_cim_namespace();
#endif
  const char *get_cim_namespace() {
    return $self->cim_ns;
  }
   
  /*
   * Set WS-Addressing reference properties
   * Argument must the string representation of a valid XML document
   *
   * call-seq:
   *   options.reference = "<xml ...>"
   */
#if defined(SWIGRUBY)
  %rename( "reference=" ) set_reference(const char *reference);
#endif
  void set_reference(const char *reference) {
    wsmc_set_reference(reference, $self);
  }

  /*
   * Get WS-Addressing reference properties
   * Returns the string representation of a valid XML document
   *
   * call-seq:
   *   options.reference -> String
   */
#if defined(SWIGRUBY)
  %rename( "reference" ) get_reference();
#endif
  const char *get_reference() {
    return $self->reference;
  }
   
  /*
   * Add an option (for OptionSet) as key/value pair
   *
   * NOTE: the value must be properly escaped (replace & with &amp;, etc.)
   *       in Ruby use CGI::escapeHTML()
   *
   * call-seq:
   *   options.add_option "Name", "Value"
   */
#if defined(SWIGRUBY)
  void add_option(VALUE k, VALUE v)
  {
    const char *key = as_string(k);
    const char *value = as_string(v);
#else
  void add_option(const char *key, const char *value)
  {
#endif
#if defined(SWIGJAVA)
    key = strdup(key);
    value = strdup(value);
#endif
    wsmc_add_option($self, key, value);
  }

#if defined(SWIGRUBY)
  /*
   * Set options (for OptionSet) from Hash
   *
   * NOTE: the values must be properly escaped (replace & with &amp;, etc.)
   *       in Ruby use CGI::escapeHTML()
   *
   * call-seq:
   *   options.options = { "Name" => "Value", ... }
   */
  %rename( "options=" ) set_options(VALUE hash);
  void set_options(VALUE hash)
  {
    $self->options = value2hash(NULL, hash, 0);
  }

  /*
   * Get options (for OptionSet) as Hash
   *
   * call-seq:
   *   options.options -> Hash
   */
  %rename( "options" ) get_options(void);
  VALUE get_options(void)
  {
    return hash2value($self->options);
  }
#endif

  /*
   * Add a selector as key/value pair
   *
   * NOTE: the value must be properly escaped (replace & with &amp;, etc.)
   *       in Ruby use CGI::escapeHTML()
   *
   * call-seq:
   *   options.add_selector "Key", "Value"
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
   *
   * NOTE: the values must be properly escaped (replace & with &amp;, etc.)
   *       in Ruby use CGI::escapeHTML()
   *
   * call-seq:
   *   options.selectors = { "Key" => "Value", ... }
   */
  %rename( "selectors=" ) set_selectors(VALUE hash);
  void set_selectors(VALUE hash)
  {
    $self->selectors = value2hash(NULL, hash, 0);
  }

  /*
   * Get selectors as Hash
   *
   * call-seq:
   *   options.selectors -> Hash
   */
  %rename( "selectors" ) get_selectors(void);
  VALUE get_selectors(void)
  {
    return hash2value($self->selectors);
  }
#endif

  /*
   * Add a property as key/value pair
   *
   * call-seq:
   *   options.add_property "Key", "Value"
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
   *
   * call-seq:
   *   options.properties = { "Key" => "Value", ...}
   */
  %rename( "properties=" ) set_properties(VALUE hash);
  void set_properties(VALUE hash)
  {
    $self->properties = value2hash(NULL, hash, 0);
  }


  /*
   * Get properties as Hash
   *
   * call-seq:
   *   options.properties -> Hash
   */
  %rename( "properties" ) get_properties(void);
  VALUE get_properties(void)
  {
    return hash2value($self->properties);
  }
#endif

  /*
   * Set delivery uri
   *
   * call-seq:
   *   options.delivery_uri = "http://..."
   */
#if defined(SWIGRUBY)
  %rename( "delivery_uri=" ) set_delivery_uri(const char *delivery_uri);
#endif
  void set_delivery_uri( const char *delivery_uri ) {
    wsmc_set_delivery_uri(delivery_uri, $self);
  }

  /*
   * Get delivery uri
   *
   * call-seq:
   *   options.delivery_uri -> String
   */
  const char *delivery_uri() {
    return $self->delivery_uri;
  }

  /*
   * Set subscription expiry timeout (in seconds)
   *
   * call-seq:
   *   options.sub_expiry = 600 # 10 mins
   */
#if defined(SWIGRUBY)
  %rename( "sub_expiry=" ) set_sub_expiry(unsigned int event_subscription_expire);
#endif
  void set_sub_expiry(unsigned int event_subscription_expire) {
	wsmc_set_sub_expiry(event_subscription_expire, $self);
  }

  /*
   * Get subscription expiry timeout (in seconds)
   *
   * call-seq:
   *   options.sub_expiry -> Integer
   */
  int sub_expiry() {
    return $self->expires;
  }

  /*
   * Set subscription heartbeat interval (in seconds)
   *
   * call-seq:
   *   options.heartbeat_interval = 60 # every minute
   */
#if defined(SWIGRUBY)
  %rename("heartbeat_interval=") set_heartbeat_interval(unsigned int heartbeat_interval);
#endif
  void set_heartbeat_interval(unsigned int heartbeat_interval) {
	wsmc_set_heartbeat_interval(heartbeat_interval, $self);
  }

  /*
   * Get subscription heartbeat interval (in seconds)
   *
   * call-seq:
   *   options.heartbeat_interval -> Integer
   */
  int heartbeat_interval() {
    return $self->heartbeat_interval;
  }

  /*
   * Set subscription delivery mode (push, pushwithack,events,pull)
   *
   * call-seq:
   *   options.delivery_mode = Openwsman::WSMAN_DELIVERY_PUSH
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

  /*
   * Get subscription delivery mode (push, pushwithack,events,pull)
   *
   * call-seq:
   *   options.delivery_mode -> Integer
   */
  int delivery_mode() {
    return $self->delivery_mode;
  }

  /*
   * Set subscription delivery security mode
   *   (auto, http basic, http digest, https basic, https digest,
   *    https mutual, https mutual basic, https mutual digest,
   *    http spnego kerberos, https spnego kerberos,
   *    https mutual spnego kerberos)
   *
   * call-seq:
   *   options.delivery_security_mode = Openwsman::WSMAN_DELIVERY_SEC_HTTPS_BASIC
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
  
  /*
   * Get subscription delivery security mode
   *
   * call-seq:
   *   options.delivery_security_mode -> Integer
   */
  int delivery_sec_mode() {
    return $self->delivery_sec_mode;
  }

}
