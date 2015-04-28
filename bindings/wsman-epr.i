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
/*struct epr_t {}; without empty struct, the %rename isn't executed. */
typedef struct {} epr_t;

/*
 * Document-class: EndPointReference
 *
 * The EndPointReference is a stub to proxy server-side operations
 *
 * Each WS-Management operation (Get, Enumerate, Invoke, ...) has an
 * associated end point reference, providing the actual implementation of
 * the operation.
 *
 */
%extend epr_t {
#if defined(SWIGRUBY)
  /*
   * Create EndPointReference from URI (String) or XML (XmlNode or XmlDoc)
   *
   * call-seq:
   *   EndPointReference.new(uri, address, { key=>value, ...})
   *   EndPointReference.new(uri)
   *   EndPointReference.new(xml)
   *
   */
  epr_t( VALUE uri, VALUE address = Qnil, VALUE selectors = Qnil) {
    KLASS_DECL(SwigClassXmlNode,SWIGTYPE_p___WsXmlNode);
    KLASS_DECL(SwigClassXmlDoc,SWIGTYPE_p__WsXmlDoc);
    if (!NIL_P(address) || !NIL_P(selectors)) {
      const char *uri_s = as_string(uri);
      const char *address_s = as_string(address);
      return epr_create(uri_s, value2hash(NULL,selectors,1), address_s);
    }
    else if (CLASS_OF(uri) == KLASS_OF(SwigClassXmlNode)) {
      WsXmlNodeH node;
      SWIG_ConvertPtr(uri, (void **)&node, SWIGTYPE_p___WsXmlNode, 0);
      return my_epr_deserialize(node);
    }
    else if (CLASS_OF(uri) == KLASS_OF(SwigClassXmlDoc)) {
      WsXmlDocH doc;
      WsXmlNodeH node;
      SWIG_ConvertPtr(uri, (void **)&doc, SWIGTYPE_p__WsXmlDoc, 0);
      node = ws_xml_get_soap_body(doc);
      if (node == NULL)
        node = ws_xml_get_doc_root(doc);
      return my_epr_deserialize(node);
    }
    else {
      return epr_from_string(as_string(uri));
    }
  }
#else
  epr_t( const char *uri, const char *address) {
    return epr_create( uri, NULL, address);
  }
  epr_t( const char *uri ) {
    return epr_from_string( uri );
  }
#endif

  ~epr_t() {
    epr_destroy( $self );
  }

#if defined(SWIGRUBY)
  %newobject clone;
  /*
   * clone the EndPointReference instance
   *
   */
  epr_t *clone(const epr_t *epr) {
    return epr_copy(epr);
  }
#endif

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
   * XML-serialize EndPointReference as child (with namespace and name) of node
   *
   */
  int serialize( WsXmlNodeH node, const char *ns, const char *epr_node_name, int embedded) {
    return epr_serialize(node, ns, epr_node_name, $self, embedded);
  }
  
#if defined(SWIGRUBY)
  %alias cmp "==";
  %typemap(out) int cmp
    "$result = ($1 == 0) ? Qtrue : Qfalse;";
#endif
  /*
   * Compare two EndPointReferences
   *
   */
  int cmp(const epr_t *epr2) {
    return epr_cmp($self, epr2);
  }
  
  /*
   * String representation (XML syntax)
   *
   */
  char *to_xml( const char *ns = NULL, const char *epr_node_name = NULL) {
    return epr_to_txt($self, ns?ns:XML_NS_ADDRESSING, epr_node_name?epr_node_name:WSA_EPR);
  }

#if defined(SWIGJAVA)
  %rename("toString") string();
#endif
#if defined(SWIGRUBY)
  %rename("to_s") string();
#endif
  %newobject string;
  /*
   * String representation (<uri>?<selector>,<selector>,...)
   *
   */
  char *string() {
    return epr_to_string($self);
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
  
#if defined(SWIGRUBY)
  /*
   * Get value of selector by name
   * epr#selector converts any value passed to String
   *
   * ==== Shortcut
   * epr.selector("name") can also be abbreviated as epr.name
   *
   * ==== Examples
   *   epr.selector("name")
   *   epr.selector(value)
   *   epr.name
   */
  char *selector(VALUE v) {
    const char *name = as_string(v);
#else
  /*
   * get value of selector by name
   *
   */
  char *selector(const char* name) {
#endif
    return wsman_epr_selector_by_name($self, name);
  }

#if !defined(SWIGJAVA) /* Target_* undefined for Java in openwsman.i */
  /*
   * Return list of selector names
   */
#if defined(SWIGRUBY)
  VALUE selector_names(void) {
#endif
#if defined(SWIGPYTHON)
  PyObject *selector_names(void) {
#endif
#if defined(SWIGPERL)
  AV *selector_names(void) {
#endif
    int i;
    Target_Type ary = Target_SizedArray($self->refparams.selectorset.count);
    key_value_t *p = $self->refparams.selectorset.selectors;
    for (i = 0; i < $self->refparams.selectorset.count; i++) {
      Target_ListSet(ary, i, SWIG_FromCharPtr(p->key));
      ++p;
    }
#if defined(SWIGPERL)
    return (AV *)ary;
#else
    return ary;
#endif
  }
#endif

#if defined(SWIGRUBY)
  /*
   * enumerate over selectors as key,value pairs
   *
   * call-seq:
   *   epr.each { |key,value| ... }
   *
   */
  void each() {
    int i;
    key_value_t *p = NULL;
    VALUE value, ary;
    p = $self->refparams.selectorset.selectors;
    for (i = 0; i < $self->refparams.selectorset.count; i++) {
      ary = rb_ary_new2(2);
      rb_ary_store(ary, 0, SWIG_FromCharPtr(p->key));
      if (p->type == 0) {
        value = SWIG_FromCharPtr(p->v.text);
      } else {
        value = SWIG_NewPointerObj((void*) p->v.epr, SWIGTYPE_p_epr_t, 0);
      }
      rb_ary_store(ary, 1, value);
      rb_yield(ary);
      p++;
    }
  }
#endif

  %newobject classname;
  /*
   * Classname of EPR
   */
  char *classname(void) {
    return uri_classname($self->refparams.uri);
  }
 
  %newobject namespace;
  /*
   * Namespace of EPR
   */
  char *namespace(void) {
    char *classname;
    int classnamelen, namespacelen;
    const char *prefix;
    const char *uri;

    /* try to get namespace from selectors (WS-Management style) */
    char *ns = get_cimnamespace_from_selectorset(&($self->refparams.selectorset));
    if (ns) {
      return strdup(ns);
    }
    /* WMI style? - extract namespace from uri */

    uri = $self->refparams.uri;
    prefix = epr_prefix(uri);
    if (prefix == NULL) {
      return NULL; /* bad classname in URI */
    }
    classname = uri_classname(uri);
    if (classname == NULL)
      return NULL; /* bad URI */
    classnamelen = strlen(classname);
    free(classname);
    namespacelen = strlen(uri) - classnamelen - strlen(prefix) - 2; /* drop enclosing slashes */
    if (namespacelen <= 0)
      return strdup("");
    /* copy after prefix slash (+ 1) */
    return strndup(uri + strlen(prefix) + 1, namespacelen);
  }
  
  %newobject prefix;
  /*
   * Prefix of EPR
   */
  char *prefix(void) {
    return epr_prefix($self->refparams.uri);
  }
}
