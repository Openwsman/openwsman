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


%extend client_opt_t {
  client_opt_t() {
    return wsmc_options_init();
  }

  ~client_opt_t() {
    wsmc_options_destroy( $self );
  }


  void set_dump_request(void) {
    wsmc_set_action_option($self, FLAG_DUMP_REQUEST );
  }

  void add_selector(char *key, char*value) {
    wsmc_add_selector($self, key, value);
  }

  void set_delivery_uri( const char *delivery_uri ) {
    wsmc_set_delivery_uri(delivery_uri, $self);
  }
	
  void set_sub_expiry(int event_subscription_expire) {
	wsmc_set_sub_expiry(event_subscription_expire, $self);
  }
	
  void set_heartbeat_interval(int heartbeat_interval) {
	wsmc_set_heartbeat_interval(heartbeat_interval, $self);
  }

  void set_delivery_mode(WsmanDeliveryMode delivery_mode) {
	wsmc_set_delivery_mode(delivery_mode, $self);
  }
}

