/*
 * wsman-epr.i
 *
 * end point reference declarations for openwsman swig bindings
 */
 
%extend epr_t {
  epr_t( const char *uri, const char *address) {
    return epr_create( uri, NULL, address);
  }

  ~epr_t() {
    epr_destroy( $self );
  }

  void add_selector(const char *name, const char *text) {
    epr_add_selector_text($self, name, text);
  }

  int serialize( WsXmlNodeH node, const char *ns, const char *epr_node_name, int embedded) {
    return epr_serialize(node, ns, epr_node_name, $self, embedded);
  }

  int cmp(epr_t *epr2) {
    return epr_cmp($self, epr2);
  }
  char *toxml( const char *ns, const char *epr_node_name) {
    return epr_to_txt($self, ns, epr_node_name);
  }

  int selector_count(void) {
    return epr_selector_count($self);
  }

  char *get_resource_uri(void) {
    return epr_get_resource_uri($self);
  }
  
  char *get_selector(const char* name) {
  	return wsman_epr_selector_by_name($self, name);
  }


}

