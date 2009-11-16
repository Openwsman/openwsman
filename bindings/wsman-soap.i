/*
 * wsman-soap.i
 *
 * soap structure accessors for openwsman swig bindings
 *
 */

%ignore __WsmanFaultCodeTable;
%ignore __WsmanFaultDetailTable;

%include "wsman-faults.h"


%nodefault __WsEnumerateInfo;
%rename(EnumerateInfo) __WsEnumerateInfo;
struct __WsEnumerateInfo {};

/*
 * EnumerateInfo contains all information related to an enumeration
 * request.
 *
 * The initial client.enumerate operation only returns a +context+
 * which can be used to +pull+ the next enumeration item. This item
 * contains the next context in the chain.
 *
 */
%extend __WsEnumerateInfo {
  ~__WsEnumerateInfo() {
  }
  int max_items() { return $self->maxItems; }
#if defined(SWIGRUBY)
  %rename("max_items=") set_max_items(int mi);
#endif
  /*
   * Set the maximum number of items returned by this enumeration
   */
  void set_max_items(int mi) { $self->maxItems = mi; }

  /*
   * flags
   */
  int flags() {
    return $self->flags;
  }
  
  /*
   * The URL of the endpoint receiving the enumeration (String)
   */
  const char *epr_to() {
    return $self->epr_to;
  }
  /*
   * The URI of the end point reference (String)
   */
  const char *epr_uri() {
    return $self->epr_uri;
  }
  /*
   * The current encoding (defaults to 'utf-8')
   */
  const char *encoding() {
    return $self->encoding;
  }
  /*
   * The Filter for this enumeration
   */
  const filter_t *filter() {
    return $self->filter;
  }
  /*
   * The current index (number of the last returned item)
   */
  int index() {
    return $self->index;
  }
#if defined(SWIGRUBY)
  %rename("index=") set_index(int i);
#endif
  /*
   * Set a specific index (used to skip ahead)
   */
  void set_index(int i) {
    $self->index = i;
  }
  /*
   * The total number of items in this enumeration
   *
   * index is the number already returned, this is the total number
   *
   */
  int total_items() {
    return $self->totalItems;
  }
#if defined(SWIGRUBY)
  %rename("total_items=") set_total_items(int i);
#endif
  void set_total_items(int i) {
    $self->totalItems = i;
  }

#if defined(SWIGRUBY)
  VALUE enum_results() {
    return (VALUE)$self->enumResults;
  }
  %rename("enum_results=") set_enum_results(VALUE result);
  void set_enum_results(VALUE result) {
    $self->enumResults = (void *)result;
  }
  VALUE enum_context() {
    return (VALUE)$self->appEnumContext;
  }
  %rename("enum_context=") set_enum_context(VALUE context);
  void set_enum_context(VALUE context) {
    $self->appEnumContext = (void *)context;
  }
#endif
  /*
   * XmlDoc representing the result pulled last
   */
  WsXmlDocH pull_result() {
    return (WsXmlDocH)$self->pullResultPtr;
  }
#if defined(SWIGRUBY)
  %rename("pull_result=") set_pull_result(WsXmlDocH result);
#endif
  /*
   * Set the pull result (XmlDoc)
   *
   * Used for server-side plugin extensions
   *
   */
  void set_pull_result(WsXmlDocH result) {
    $self->pullResultPtr = (void *)result;
  }
}


/*
 * __SoapOp -> SoapOp
 */
 
%nodefault __SoapOp;
%rename(SoapOp) __SoapOp;
struct __SoapOp {};

/*
 * SoapOp represents a SOAP operation
 *
 */
%extend __SoapOp {
  /*
   * The incoming XmlDoc
   */
  WsXmlDocH indoc() {
    return soap_get_op_doc($self, 1);
  }
#if defined(SWIGRUBY)
  %rename("indoc=") set_indoc( WsXmlDocH doc );
#endif
  /*
   * Set the incoming XmlDoc
   */
  void set_indoc( WsXmlDocH doc ) {
    soap_set_op_doc( $self, doc, 1 );
  }
  /*
   * The outgoing XmlDoc
   */
  WsXmlDocH outdoc() {
    return soap_get_op_doc($self, 0);
  }
#if defined(SWIGRUBY)
  %rename("outdoc=") set_outdoc( WsXmlDocH doc );
#endif
  /*
   * Set the outgoing XmlDoc
   */
  void set_outdoc( WsXmlDocH doc ) {
    soap_set_op_doc( $self, doc, 0 );
  }
  /*
   * The Soap instance of this operation
   */
  struct __Soap *soap() {
    return soap_get_op_soap($self);
  }
  /*
   * The raw (SOAP) message for this operation (opaque pointer
   * currently)
   */
  WsmanMessage *msg() {
    return wsman_get_msg_from_op($self);
  }
  /*
   * The maximum size (on the wire) of this operation
   */
  unsigned long maxsize(){
    return wsman_get_maxsize_from_op($self);
  }
}


/*
 * __Soap -> Soap
 */
 
%nodefault __Soap;
%rename(Soap) __Soap;
struct __Soap {};

/*
 * Soap represents a part of a SoapOp used to create and reference
 * context information.
 *
 * There is no constructor for Soap, use SoapOp.soap to access the
 * Soap instance associated with a SOAP operation.
 *
 */
%extend __Soap {
  ~__Soap() {
    soap_destroy($self);
  }
  %typemap(newfree) WsContextH "free($1);";

  /*
   * Create a new Context
   *
   */
  WsContextH create_context() {
    return ws_create_context($self);
  }
  /*
   * Get the current Context
   *
   */
  WsContextH context() {
    return ws_get_soap_context($self);
  }

  /*
   * Create a new endpoint Context
   *
   */
  WsContextH create_ep_context( WsXmlDocH doc ) {
    return ws_create_ep_context( $self, doc );
  }
}


/*
 * WsContext -> Context
 */
 
%rename(Context) _WS_CONTEXT;
%nodefault _WS_CONTEXT;
struct _WS_CONTEXT {};
typedef struct _WS_CONTEXT* WsContextH;


/*
 * Context contains all information of an ongoing SOAP operation
 *
 * There is no constructor for Context, use Soap.context to get the
 * current one.
 *
 */
%extend _WS_CONTEXT {
  ~_WS_CONTEXT() {
    ws_destroy_context($self);
  }
  %typemap(newfree) WsXmlDocH "ws_xml_destroy_doc($1);";
  /*
   * The incoming XmlDoc
   */
  WsXmlDocH indoc() {
    return $self->indoc;
  }
  /*
   * Get the Soap runtime environment
   *
   */
  struct __Soap *runtime() {
    return ws_context_get_runtime($self);
  }
#if defined(SWIGRUBY)
  %rename("enum_idle_timeout=") set_enumIdleTimeout(unsigned long timeout);
#endif
  /*
   * Set the idle timeout for enumerations
   */
  void set_enumIdleTimeout(unsigned long timeout) {
    ws_set_context_enumIdleTimeout($self, timeout);
  }
  /*
   * The class name (String)
   */
  const char *classname() {
    return wsman_get_class_name($self);
  }
  /*
   * The method name (String)
   */
  const char *method() {
    return wsman_get_method_name($self);
  }
  /*
   * The method arguments (Hash)
   */
  hash_t *method_args(const char *resource_uri) {
    return wsman_get_method_args($self, resource_uri);
  }
  int max_elements(WsXmlDocH doc = NULL) {
    return wsman_get_max_elements($self, doc);
  }
  unsigned long max_envelope_size(WsXmlDocH doc = NULL) {
    return wsman_get_max_envelope_size($self, doc);
  }
  const char *fragment_string(WsXmlDocH doc = NULL) {
    return wsman_get_fragment_string($self, doc);
  }
  const char *selector(WsXmlDocH doc, const char *name, int index) {
    return wsman_get_selector($self, doc, name, index);
  }
  hash_t *selectors_from_epr(WsXmlNodeH epr_node) {
    return wsman_get_selectors_from_epr($self, epr_node);
  }
  hash_t *selectors(WsXmlDocH doc = NULL) {
    return wsman_get_selector_list($self, doc);
  }
  hash_t *selectors_from_filter(WsXmlDocH doc = NULL) {
    return wsman_get_selector_list_from_filter($self, doc);
  }
  const char *action(WsXmlDocH doc = NULL) {
    return wsman_get_action($self, doc);
  }
  const char *resource_uri(WsXmlDocH doc = NULL) {
    return wsman_get_resource_uri($self, doc);
  }
  const char *option_set(WsXmlDocH doc, const char *op) {
    return wsman_get_option_set($self, doc, op);
  }
  int parse_enum_request(WsEnumerateInfo *enumInfo) {
    return wsman_parse_enum_request( $self, enumInfo);
  }
  
}


/*
 * WsmanStatus -> Status
 */
 
%nodefault _WsmanStatus;
%rename(Status) _WsmanStatus;
struct _WsmanStatus {};

/*
 * Status
 *
 */
%extend _WsmanStatus {
  /*
   * Create an empty status
   *
   */
  _WsmanStatus() {
    WsmanStatus *s = (WsmanStatus *)malloc(sizeof(WsmanStatus));
    wsman_status_init(s);
    return s;
  }
  ~_WsmanStatus() {
    if ($self->fault_msg) free($self->fault_msg);
    free($self);
  }
  /*
   * String representation (returns the fault message)
   */
  const char *to_s() {
    return $self->fault_msg;
  }
#if defined(SWIGRUBY)
  %rename("code=") set_code(int code);
#endif
  /*
   * Set the fault code
   */
  void set_code(int code) { $self->fault_code = code; }
  /*
   * Get the fault code
   */
  int code() {
    return $self->fault_code;
  }
#if defined(SWIGRUBY)
  %rename("detail=") set_detail(int detail);
#endif
  /*
   * Set the fault detail code
   *
   */
  void set_detail(int detail) { $self->fault_detail_code = detail; }
  int detail() {
    return $self->fault_detail_code;
  }
#if defined(SWIGRUBY)
  %rename("msg=") set_msg(const char *msg);
#endif
  /*
   * Set the fault message
   *
   */
  void set_msg(const char *msg) {
    if (msg)
      $self->fault_msg = strdup(msg);
    else
      $self->fault_msg = NULL;
  }
  const char *msg() {
    return $self->fault_msg;
  }

  %typemap(newfree) WsXmlDocH "ws_xml_destroy_doc($1);";
  /*
   * Create a new fault XmlDoc
   *
   */
  WsXmlDocH generate_fault(WsXmlDocH doc) {
    return wsman_generate_fault( doc, $self->fault_code, $self->fault_detail_code, $self->fault_msg);
  }
}

/*
 * WsManFault -> Fault
 */
 
%nodefault _WsManFault;
%rename(Fault) _WsManFault;
struct _WsManFault {};

/*
 * Fault
 *
 */
%extend _WsManFault {
  _WsManFault() {
    return wsmc_fault_new();
  }
  ~_WsManFault() {
    wsmc_fault_destroy($self);
  }
  const char *code() {
    return $self->code;
  }
  const char *subcode() {
    return $self->subcode;
  }
  const char *reason() {
    return $self->reason;
  }
  const char *detail() {
    return $self->fault_detail;
  }
}
