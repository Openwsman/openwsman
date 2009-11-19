/*
 * wsman-xml.i
 * xml structure accessors for openwsman swig bindings
 *
 */

/*
 * XmlNs
 * Xml namespace
 */
 
%rename(XmlNs) __WsXmlNs;
%nodefault __WsXmlNs;   /* part of WsXmlAttr */
struct __WsXmlNs {}; /* without empty struct, the %rename isn't executed. */
typedef struct __WsXmlNs* WsXmlNsH;

/*
 * class XmlDoc
 *
 * Implementation advice
 *  DONT do a %newobject on functions returning WsXmlDoc. Swig will
 * free the WsXmlDocH immediately after wrapping !
 */

%rename(XmlDoc) _WsXmlDoc;
%nodefault _WsXmlDoc;
struct _WsXmlDoc {};
typedef struct _WsXmlDoc* WsXmlDocH;

/*
 * Document-class: XmlDoc
 *
 * XmlDoc holds an XML document and thus represents the root of an XML
 * tree. XmlDoc is optimized for SOAP type documents, giving accessors
 * to the SOAP envelope, header and body.
 *
 * Instances of the other XML related classes like XmlAttr and XmlNode
 * can only be created with an associated XmlDoc instance.
 *
 * Main properties of the XML document are
 * * name of the root element
 * * encoding (defaults to _UTF-8_)
 *
 */
%extend _WsXmlDoc {
  /*
   * Create XmlDoc with node name
   * optionally pass namespace as 2nd arg (defaults to NULL)
   */
  _WsXmlDoc(const char *name, const char *ns = NULL) {
    return ws_xml_create_doc(ns, name);
  }
  /* destructor */
  ~_WsXmlDoc() {
    ws_xml_destroy_doc( $self );
  }
  %typemap(newfree) char * "free($1);";
#if defined(SWIGRUBY)
  %alias string "to_s";
#endif
#if defined(SWIGPYTHON)
  %rename("__str__") string();
#endif
  
  %newobject string;
  /*
   * generic string representation of the XmlDoc
   */
  char *string() {
    int size;
    char *buf;
    ws_xml_dump_memory_node_tree( ws_xml_get_doc_root($self), &buf, &size );
    return buf;
  }

  /*
   * encode document as string with specific encoding
   *
   * encoding defaults to 'utf-8'
   *
   */
  %newobject encode;
  char *encode(const char *encoding="utf-8") {
    int size;
    char *buf;
    ws_xml_dump_memory_enc( $self, &buf, &size, encoding );
    return buf;
  }
  /*
   * dump document to file
   */
  void dump_file(FILE *fp) {
    ws_xml_dump_doc( fp, $self );
  }			      
  /*
   * get root node of doc
   * call-seq:
   *  doc.root -> XmlNode
   *
   */
  WsXmlNodeH root() {
    return ws_xml_get_doc_root( $self );
  }
  /*
   * get soap envelope node
   * call-seq:
   *  doc.envelope -> XmlNode
   *
   */
  WsXmlNodeH envelope() {
    return ws_xml_get_soap_envelope( $self );
  }
  /*
   * get soap header node
   * call-seq:
   *  doc.header -> XmlNode
   *
   */
  WsXmlNodeH header() {
    return ws_xml_get_soap_header( $self );
  }
  /*
   * get soap body node
   * call-seq:
   *  doc.body -> XmlNode
   *
   */
  WsXmlNodeH body() {
    return ws_xml_get_soap_body( $self );
  }
  /*
   * get soap element node by name
   * returns nil if no element with the name can be found
   *
   */
  WsXmlNodeH element(const char *name) {
    return ws_xml_get_soap_element( $self, name );
  }
  /*
   * get enumeration context as string
   * call-seq:
   *  doc.context -> String
   *
   */
  const char *context() {
    return wsmc_get_enum_context( $self );
  }
  /*
   * Generate fault document based on given status
   *
   * This creates a new XmlDoc instance representing a fault
   *
   */
  WsXmlDocH generate_fault(WsmanStatus *s) {
    return wsman_generate_fault( $self, s->fault_code, s->fault_detail_code, s->fault_msg);
  }
  
#if defined(SWIGRUBY)
  %rename("fault?") is_fault();
  %typemap(out) int is_fault
    "$result = ($1 != 0) ? Qtrue : Qfalse;";
#endif
  /*
   * Check if document represents a fault
   *
   */
  int is_fault() {
    return wsmc_check_for_fault( $self );
  }
  
  /*
   * retrieve fault data
   */
  %newobject fault;
  WsManFault *fault() {
    WsManFault *f = NULL;
    if (wsmc_check_for_fault($self)) {
      f = (WsManFault *)calloc(1, sizeof(WsManFault));
      wsmc_get_fault_data($self, f);
    }
    return f;
  }
  
  /*
   * Generate response envelope document, optionally relating to a
   * specific action.
   *
   * This creates a new XmlDoc instance representing a response.
   *
   */
  WsXmlDocH create_response_envelope(const char *action = NULL) {
    return wsman_create_response_envelope($self, action);
  }
  
#if defined(SWIGRUBY)
  %rename("end_of_sequence?") is_end_of_sequence();
  %typemap(out) int is_end_of_sequence
    "$result = ($1 != 0) ? Qtrue : Qfalse;";
#endif
  /*
   * Check if document represents an end of sequence (last enumeration item)
   *
   */
  int is_end_of_sequence() {
    return NULL != ws_xml_find_in_tree( ws_xml_get_soap_body( $self ), XML_NS_ENUMERATION, WSENUM_END_OF_SEQUENCE, 1 );
  }

}


/*
 * Document-class: XmlNode
 *
 * XmlNode is a node inside the XML document tree.
 * 
 * A node has
 * * a name
 * * a namespace (optional)
 * * attributes
 * * text (optional)
 * * a parent
 * * a document (root)
 * * children (empty for tail nodes)
 *
 */
 
%rename(XmlNode) __WsXmlNode;
%nodefault __WsXmlNode;
struct __WsXmlNode {}; /* without empty struct, the %rename isn't executed. */
typedef struct __WsXmlNode* WsXmlNodeH;

%extend __WsXmlNode {
  ~__WsXmlNode() {
    ws_xml_unlink_node($self);
  }
#if defined(SWIGRUBY)
  %alias text "to_s";
#endif
#if defined(SWIGPYTHON)
  %rename("__str__") text();
#endif

  %newobject string;
  /*
   * dump node as XML string
   */
  char *string() {
    int size;
    char *buf;
    ws_xml_dump_memory_node_tree( $self, &buf, &size );
    return buf;
  }
  
  /*
   * dump node to file
   */
  void dump_file(FILE *fp) {
    ws_xml_dump_node_tree( fp, $self );
  }
  
#if defined(SWIGRUBY)
  %alias equal "==";
  %typemap(out) int equal
    "$result = ($1 != 0) ? Qtrue : Qfalse;";
#endif
#if defined(SWIGPERL)
  int __eq__( WsXmlNodeH n )
#else
  int equal( WsXmlNodeH n )
#endif
  { return $self == n; }	  
  
  /*
   * get text (without xml tags) of node
   */
  char *text() {
    return ws_xml_get_node_text( $self );
  }
#if defined(SWIGRUBY)
  %rename( "text=" ) set_text( const char *text );
#endif
  /*
   * Set text of node
   */
  void set_text( const char *text ) {
    ws_xml_set_node_text( $self, text );
  }
  
  /*
   * get XmlDoc to which node belongs
   */
  WsXmlDocH doc() {
    return ws_xml_get_node_doc( $self );
  }
  
  /*
   * get parent for node
   */
  WsXmlNodeH parent() {
    return ws_xml_get_node_parent( $self );
  }
#if defined(SWIGRUBY)
  %alias child "first";
#endif

  /*
   * get first child of node
   */
  WsXmlNodeH child() {
    if( ws_xml_get_child_count( $self ) > 0 )
      return ws_xml_get_child($self, 0, NULL, NULL);
    return NULL;
  }
  
  /*
   * get name for node
   */
  char *name() {
    return ws_xml_get_node_local_name( $self );
  }
#if defined(SWIGRUBY)
  %rename("name=") set_name( const char *name);
#endif

  /*
   * set name of node
   */
  void set_name( const char *name ) {
    ws_xml_set_node_name( $self, ws_xml_get_node_name_ns( $self ), name );
  }
  
  /*
   * get namespace for node
   */
  char *ns() {
    return ws_xml_get_node_name_ns( $self );
  }
#if defined(SWIGRUBY)
  %rename("ns=") set_ns( const char *nsuri );
#endif

  /*
   * set namespace of node
   */
  void set_ns( const char *ns ) {
    ws_xml_set_ns( $self, ns, ws_xml_get_node_name_ns_prefix($self) );
  }

  /*
   * get prefix of nodes namespace
   */
  const char *prefix() {
    return ws_xml_get_node_name_ns_prefix($self);
  }

  /*
   * set language
   */
#if defined(SWIGRUBY)
  %rename("lang=") set_lang(const char *lang);
#endif
  void set_lang(const char *lang) {
    ws_xml_set_node_lang($self, lang);
  }

  /*
   * find node within tree
   * a NULL passed as 'ns' (namespace) is treated as wildcard
   */
  WsXmlNodeH find( const char *ns, const char *name, int recursive = 1) {
    return ws_xml_find_in_tree( $self, ns, name, recursive );
  }
				 
  /*
   * count node children
   */
  int size() {
    return ws_xml_get_child_count( $self );
  }
  
  /*
   * add child (namespace, name, text) to node
   */
  WsXmlNodeH add( const char *ns, const char *name, const char *text = NULL ) {
    return ws_xml_add_child( $self, ns, name, text );
  }

  /*
   * add child (namespace, name, text) before(!) node
   */
  WsXmlNodeH add_before( const char *ns, const char *name, const char *text = NULL ) {
    return ws_xml_add_prev_sibling( $self, ns, name, text );
  }

#if defined(SWIGRUBY)
  %alias add "<<";
#endif
  /*
   * add node as child
   */
  WsXmlNodeH add(WsXmlNodeH node) {
    ws_xml_duplicate_tree( $self, node );
    return $self;
  }
  
  /*
   * iterate children
   */

#if defined(SWIGRUBY)
  void each() {
    int i = 0;
    while ( i < ws_xml_get_child_count( $self ) ) {
      rb_yield( SWIG_NewPointerObj((void*) ws_xml_get_child($self, i, NULL, NULL), SWIGTYPE_p___WsXmlNode, 0));
      ++i;
    }
  }
#endif

#if defined(SWIGPYTHON)
  %pythoncode %{
    def __iter__(self):
      r = range(0,self.size())
      while r:
        yield self.get(r.pop(0))
  %}
#endif

#if defined(SWIGRUBY)
  %alias get "[]";
#endif
  /*
   * get child by index
   */
  WsXmlNodeH get(int i) {
    if (i < 0 || i >= ws_xml_get_child_count($self))
      return NULL;
    return ws_xml_get_child($self, i, NULL, NULL);
  }
  
  /*
   * get child by name
   */
  WsXmlNodeH get(const char *name) {
    int i = 0;
    while ( i < ws_xml_get_child_count($self)) {
      WsXmlNodeH child = ws_xml_get_child($self, i, NULL, NULL);
      if (!strcmp(ws_xml_get_node_local_name(child), name))
        return child;
      ++i;
    }
    return NULL;
  }

  /* get node attribute */
  WsXmlAttrH attr(int index = 0) {
    return ws_xml_get_node_attr( $self, index );
  }
  /* count node attribute */
  int attr_count() {
    return ws_xml_get_node_attr_count( $self );
  }
  /* find node attribute by name */
  WsXmlAttrH attr_find( const char *ns, const char *name ) {
    return ws_xml_find_node_attr( $self, ns, name );
  }
  /* add attribute to node */
  WsXmlAttrH attr_add( const char *ns, const char *name, const char *value ) {
    return ws_xml_add_node_attr( $self, ns, name, value );
  }

  epr_t *epr( const char *ns, const char *epr_node_name, int embedded) {
    return epr_deserialize($self, ns, epr_node_name, embedded);
  }  


#if defined(SWIGRUBY)
  /* enumerate attributes */
  void each_attr() {
    int i = 0;
    while ( i < ws_xml_get_node_attr_count( $self ) ) {
      rb_yield( SWIG_NewPointerObj((void*) ws_xml_get_node_attr($self, i), SWIGTYPE_p___WsXmlAttr, 0));
      ++i;
    }
  }
#endif
}


/*
 * Document-class: XmlAttr
 * An XmlAttr is a key/value pair representing an attribute of a node.
 *
 * An attribute has
 * * a name (the key)
 * * a namespace (optional)
 * * a value
 *
 * There is no standalone constructor available for XmlAttr, use
 * XmlNode.add_attr() to create a new attribute.
 *
 */
 
%rename(XmlAttr) __WsXmlAttr;
%nodefault __WsXmlAttr; /* part of WsXmlNode */
struct __WsXmlAttr {}; /* without empty struct, the %rename isn't executed. */
typedef struct __WsXmlAttr* WsXmlAttrH;

%extend __WsXmlAttr {
#if defined(SWIGRUBY)
  %alias value "to_s";
#endif
  /* get name for attr */
  char *name() {
    return ws_xml_get_attr_name( $self );
  }
  /* get namespace for attr */
  char *ns() {
    return ws_xml_get_attr_ns( $self );
  }
  /* get value for attr */
  char *value() {
    return ws_xml_get_attr_value( $self );
  }
  /* remove note attribute */
  void remove() {
    ws_xml_remove_node_attr( $self );
  }
}

