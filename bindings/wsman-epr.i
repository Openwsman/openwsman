/*
 * wsman-epr.i
 *
 * end point reference declarations for openwsman swig bindings
 *
 * EndPointReference
 *
 */
 
 
%rename(EndPointReference) epr_t;
%nodefault epr_t;
typedef struct {
    char * address;
} epr_t;

/*
 * EndPointReference
 *
 * The EndPointReference is a stub to proxy server-side operations
 *
 * Each WS-Management operation (Get, Enumerate, Invoke, ...) has an
 * associated end point reference, providing the actual implementation of
 * the operation.
 *
 */
%extend epr_t {
  epr_t( const char *uri, const char *address) {
    return epr_create( uri, NULL, address);
  }

  ~epr_t() {
    epr_destroy( $self );
  }

  /*
   * Add selector as key/value pair
   *
   */
  void add_selector(const char *name, const char *text) {
    epr_add_selector_text($self, name, text);
  }

  /*
   * Serialization
   *
   */
  int serialize( WsXmlNodeH node, const char *ns, const char *epr_node_name, int embedded) {
    return epr_serialize(node, ns, epr_node_name, $self, embedded);
  }
  
  /*
   * Compare two EndPointReferences
   *
   */
  int cmp(epr_t *epr2) {
    return epr_cmp($self, epr2);
  }
  
  /*
   * String representation (XML syntax)
   *
   */
  char *to_xml( const char *ns, const char *epr_node_name) {
    return epr_to_txt($self, ns, epr_node_name);
  }

  /*
   * Number of selectors
   */
  int selector_count(void) {
    return epr_selector_count($self);
  }

  /*
   * The resource URI associated to this EndPointReference
   *
   */
  char *resource_uri(void) {
    return epr_get_resource_uri($self);
  }
  
  /*
   * get value of selector by name
   *
   */
  char *selector(const char* name) {
  	return wsman_epr_selector_by_name($self, name);
  }

}

