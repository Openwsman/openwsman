/*
 *
 * ClientOptions
 *
 * client option declarations for openwsman swig bindings
 *
 */
 
%rename(ClientOptions) client_opt_t;
%nodefault client_opt_t;
typedef struct {
} client_opt_t;


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
   * Add a selector as key/value pair
   *
   */
  void add_selector(char *key, char *value) {
    wsmc_add_selector($self, key, value);
  }

#if defined(SWIGRUBY)
  %rename("delivery_uri=") set_delivery_uri();
#endif
  void set_delivery_uri( const char *delivery_uri ) {
    wsmc_set_delivery_uri(delivery_uri, $self);
  }
	
#if defined(SWIGRUBY)
  %rename("sub_expiry=") set_sub_expiry();
#endif
  void set_sub_expiry(int event_subscription_expire) {
	wsmc_set_sub_expiry(event_subscription_expire, $self);
  }
	
#if defined(SWIGRUBY)
  %rename("heartbeat_interval=") set_heartbeat_interval();
#endif
  void set_heartbeat_interval(int heartbeat_interval) {
	wsmc_set_heartbeat_interval(heartbeat_interval, $self);
  }

#if defined(SWIGRUBY)
  %rename("delivery_mode=") set_delivery_mode();
#endif
  void set_delivery_mode(WsmanDeliveryMode delivery_mode) {
	wsmc_set_delivery_mode(delivery_mode, $self);
  }
}

