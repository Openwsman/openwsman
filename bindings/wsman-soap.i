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
 * Document-class: EnumerateInfo
 *
 * EnumerateInfo contains all information related to an enumeration
 * request.
 *
 * The initial client.enumerate operation only returns a +context+
 * (String) which can be used to +pull+ the next enumeration item.
 * This item contains the next context in the chain.
 *
 */
%extend __WsEnumerateInfo {
  ~__WsEnumerateInfo() {
  }
  /*
   * Return the maximum number of items that will be returned by this enumeration
   * call-seq:
   *    enumerate_info.max_items -> Integer
   *
   */
  int max_items() { return $self->maxItems; }
#if defined(SWIGRUBY)
  %rename("max_items=") set_max_items(int mi);
#endif
  /*
   * Set the maximum number of items returned by this enumeration
   * call-seq:
   *    enumerate_info.max_items = 100
   *
   */
  void set_max_items(int mi) { $self->maxItems = mi; }

  /*
   * enumeration flags
   * call-seq:
   *    enumerate_info.flags -> Integer
   *
   */
  int flags() {
    return $self->flags;
  }
  
  /*
   * The URL of the endpoint receiving the enumeration
   * call-seq:
   *    enumerate_info.epr_to -> String
   *
   */
  const char *epr_to() {
    return $self->epr_to;
  }
  /*
   * The URI of the end point reference
   * call-seq:
   *    enumerate_info.epr_uri -> String
   *
   */
  const char *epr_uri() {
    return $self->epr_uri;
  }
  /*
   * The current encoding (defaults to 'utf-8')
   * call-seq:
   *    enumerate_info.encoding -> String
   *
   */
  const char *encoding() {
    return $self->encoding;
  }
  /*
   * The Filter for this enumeration
   * call-seq:
   *    enumerate_info.filter -> Openwsman::Filter
   *
   */
  const filter_t *filter() {
    return $self->filter;
  }
  /*
   * The current index (number of the last returned item)
   * call-seq:
   *    enumerate_info.index -> Integer
   *
   */
  int index() {
    return $self->index;
  }
#if defined(SWIGRUBY)
  %rename("index=") set_index(int i);
#endif
  /*
   * Set a specific index (used to skip ahead)
   * call-seq:
   *    enumerate_info.index = 42
   *
   */
  void set_index(int i) {
    $self->index = i;
  }
  /*
   * The total number of items in this enumeration
   *
   * index is the number already returned, this is the total number
   * call-seq:
   *    enumerate_info.total_items -> Integer
   *
   */
  int total_items() {
    return $self->totalItems;
  }
#if defined(SWIGRUBY)
  %rename("total_items=") set_total_items(int i);
#endif
  /*
   * Set the total number of items in this enumeration
   * call-seq:
   *    enumerate_info.total_items = 10
   *
   */
  void set_total_items(int i) {
    $self->totalItems = i;
  }

  /*
   * XmlDoc representing the result pulled last
   * call-seq:
   *    enumerate_info.pull_result -> Openwsman::XmlDoc
   *
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
   * call-seq:
   *    enumerate_info.pull_result = xml_doc
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
 * Document-class: SoapOp
 *
 * SoapOp represents a SOAP operation
 *
 */
%extend __SoapOp {
  /*
   * The incoming XmlDoc
   * call-seq:
   *   soap_op.indoc -> Openwsman::XmlDoc
   *
   */
  WsXmlDocH indoc() {
    return soap_get_op_doc($self, 1);
  }
#if defined(SWIGRUBY)
  %rename("indoc=") set_indoc( WsXmlDocH doc );
#endif
  /*
   * Set the incoming XmlDoc
   * call-seq:
   *   soap_op.indoc = xml_doc
   *
   */
  void set_indoc( WsXmlDocH doc ) {
    soap_set_op_doc( $self, doc, 1 );
  }
  /*
   * The outgoing XmlDoc
   * call-seq:
   *   soap_op.outdoc -> Openwsman::XmlDoc
   *
   */
  WsXmlDocH outdoc() {
    return soap_get_op_doc($self, 0);
  }
#if defined(SWIGRUBY)
  %rename("outdoc=") set_outdoc( WsXmlDocH doc );
#endif
  /*
   * Set the outgoing XmlDoc
   * call-seq:
   *   soap_op.outdoc = xml_doc
   *
   */
  void set_outdoc( WsXmlDocH doc ) {
    soap_set_op_doc( $self, doc, 0 );
  }
  /*
   * The Soap instance of this operation
   * call-seq:
   *   soap_op.soap -> Openwsman::Soap
   *
   */
  struct __Soap *soap() {
    return soap_get_op_soap($self);
  }
  /*
   * The raw (SOAP) message for this operation
   * call-seq:
   *   soap_op.msg -> Openwsman::Message
   *
   */
  WsmanMessage *msg() {
    return wsman_get_msg_from_op($self);
  }
  /*
   * The maximum size (on the wire) of this operation
   * call-seq:
   *   soap_op.maxsize -> Integer
   *
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
 * Document-class: Soap
 *
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
   * call-seq:
   *   soap.create_context -> Openwsman::Context
   *
   */
  WsContextH create_context() {
    return ws_create_context($self);
  }
  /*
   * Get the current Context
   * call-seq:
   *   soap.context -> Openwsman::Context
   *
   */
  WsContextH context() {
    return ws_get_soap_context($self);
  }

  /*
   * Create a new endpoint Context
   * call-seq:
   *   soap.create_ep_context -> Openwsman::Context
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
 * Document-class: Context
 *
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
   * call-seq:
   *   context.indox -> Openwsman::XmlDoc
   *
   */
  WsXmlDocH indoc() {
    return $self->indoc;
  }
  /*
   * Get the Soap runtime environment
   * call-seq:
   *   context.runtime -> Openwsman::Soap
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
   * call-seq:
   *   context.enum_idle_timeout = 60
   *
   */
  void set_enumIdleTimeout(unsigned long timeout) {
    ws_set_context_enumIdleTimeout($self, timeout);
  }
  /*
   * The class name
   * call-seq:
   *   context.classname -> String
   *
   */
  const char *classname() {
    return wsman_get_class_name($self);
  }
  /*
   * The method name
   * call-seq:
   *   context.method -> String
   *
   */
  const char *method() {
    return wsman_get_method_name($self);
  }
  /*
   * The method arguments
   * call-seq:
   *   context.method -> Hash
   *
   */
  hash_t *method_args(const char *resource_uri) {
    return wsman_get_method_args($self, resource_uri);
  }
  /*
   * The maximum elements of the document
   * call-seq:
   *   context.max_elements -> Integer
   *   context.max_elements(xmldoc) -> Integer
   *
   */
  int max_elements(WsXmlDocH doc = NULL) {
    return wsman_get_max_elements($self, doc);
  }
  /*
   * The maximum envelope size of the document
   * call-seq:
   *   context.max_envelope_size -> Integer
   *   context.max_envelope_size(xmldoc) -> Integer
   *
   */
  unsigned long max_envelope_size(WsXmlDocH doc = NULL) {
    return wsman_get_max_envelope_size($self, doc);
  }
  /*
   * The fragment of the document
   * call-seq:
   *   context.fragment_string -> String
   *   context.fragment_string(xmldoc) -> String
   *
   */
  const char *fragment_string(WsXmlDocH doc = NULL) {
    return wsman_get_fragment_string($self, doc);
  }
  /*
   * The selector for an element
   * call-seq:
   *   context.selector(xml_doc, "Name", 1) -> String
   *
   */
  const char *selector(WsXmlDocH doc, const char *name, int index) {
    return wsman_get_selector($self, doc, name, index);
  }
  /*
   * The selectors from an endpoint reference
   * call-seq:
   *   context.selectors_from_epr(epr_node_as_xml_doc) -> Hash
   *
   */
  hash_t *selectors_from_epr(WsXmlNodeH epr_node) {
    return wsman_get_selectors_from_epr($self, epr_node);
  }
  /*
   * The selectors for a document
   * call-seq:
   *   context.selectors -> Hash
   *   context.selectors(xml_doc) -> Hash
   *
   */
  hash_t *selectors(WsXmlDocH doc = NULL) {
    return wsman_get_selector_list($self, doc);
  }
  /*
   * The selectors from a filter
   * call-seq:
   *   context.selectors_from_filter -> Hash
   *   context.selectors_from_filter(xml_doc) -> Hash
   *
   */
  hash_t *selectors_from_filter(WsXmlDocH doc = NULL) {
    return wsman_get_selector_list_from_filter($self, doc);
  }
  /*
   * The action
   * call-seq:
   *   context.action -> String
   *   context.action(xml_doc) -> String
   *
   */
  const char *action(WsXmlDocH doc = NULL) {
    return wsman_get_action($self, doc);
  }
  /*
   * The resource uri
   * call-seq:
   *   context.resource_uri -> String
   *   context.resource_uri(xml_doc) -> String
   *
   */
  const char *resource_uri(WsXmlDocH doc = NULL) {
    return wsman_get_resource_uri($self, doc);
  }
  /*
   * The option set
   * call-seq:
   *   context.option_set(xml_doc, "op") -> String
   *
   */
  const char *option_set(WsXmlDocH doc, const char *op) {
    return wsman_get_option_set($self, doc, op);
  }
  /*
   * Parse enumeration request
   * call-seq:
   *   context.parse_enum_request(enumerate_info) -> Integer
   *
   */
  int parse_enum_request(WsEnumerateInfo *enumInfo) {
    WsmanStatus status;
    wsman_status_init(&status);
    return wsman_parse_enum_request( $self, enumInfo, &status);
    /* FIXME: return status */
  }
  
}


/*
 * WsmanStatus -> Status
 */

%rename(Status) _WsmanStatus;
%nodefault _WsmanStatus;
struct _WsmanStatus {};
typedef struct _WsmanStatus WsmanStatus;

/*
 * Document-class: Status
 *
 * Status represents the detailed status of a (failed) WS-Management
 * operation.
 *
 * Its primarily used implementing server-side plugins to report a Fault
 * back to the calling client.
 *
 */
%extend _WsmanStatus {
  /*
   * Create a new Status object
   *
   * call-seq:
   *   Openwsman::Status.new # create 'good' status
   *   Openwsman::Status.new error_code
   *   Openwsman::Status.new error_code, error_detail
   *   Openwsman::Status.new error_code, error_detail, "Error message"
   *
   */
  _WsmanStatus(int code = 0, int detail = 0, const char *msg = NULL) {
    WsmanStatus *s = (WsmanStatus *)malloc(sizeof(WsmanStatus));
    wsman_status_init(s);
    if (code)
      s->fault_code = code;
    if (msg)
      s->fault_msg = strdup(msg);
    if (detail < 0
        || detail > OWSMAN_SYSTEM_ERROR) {
      SWIG_exception( SWIG_ValueError, "Bad fault detail" );
    }
    else {
      s->fault_detail_code = detail;
    }
#if defined(SWIGPYTHON) || defined(SWIGPERL) || defined(SWIGJAVA)
    fail:
#endif
    return s;
  }
  ~_WsmanStatus() {
    if ($self->fault_msg) free($self->fault_msg);
    free($self);
  }
  /*
   * String representation (returns the fault message)
   *
   * call-seq:
   *   status.to_s -> String
   *
   */
  const char *to_s() {
    return $self->fault_msg;
  }
#if defined(SWIGRUBY)
  %rename("code=") set_code(int code);
#endif
  /*
   * Set the fault code
   *
   * call-seq:
   *   status.code = 1
   *
   */
  void set_code(int code) { $self->fault_code = code; }
  /*
   * Get the fault code
   *
   * call-seq:
   *   status.code -> Integer
   *
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
   * call-seq:
   *   status.detail = 42
   *
   */
  void set_detail(int detail) {
    if (detail < 0
        || detail > OWSMAN_SYSTEM_ERROR) {
      SWIG_exception( SWIG_ValueError, "Bad fault detail" );
    }
    else {
      $self->fault_detail_code = detail;
    }
#if defined(SWIGPYTHON) || defined(SWIGPERL) || defined(SWIGJAVA)
    fail:
    return;
#endif
  }
  /*
   * Get the fault detail code
   *
   * call-seq:
   *   status.detail -> Integer
   *
   */
  int detail() {
    return $self->fault_detail_code;
  }
#if defined(SWIGRUBY)
  %rename("msg=") set_msg(const char *msg);
#endif
  /*
   * Set the fault message
   *
   * call-seq:
   *   status.msg = "This is a fault message"
   *
   */
  void set_msg(const char *msg) {
    if ($self->fault_msg)
      free($self->fault_msg);
    if (msg)
      $self->fault_msg = strdup(msg);
    else
      $self->fault_msg = NULL;
  }
  /*
   * Get the fault message
   *
   * call-seq:
   *   status.msg -> String
   *
   */
  const char *msg() {
    return $self->fault_msg;
  }

  %typemap(newfree) WsXmlDocH "ws_xml_destroy_doc($1);";
  /*
   * Create a new fault XmlDoc based on Status information
   *
   * call-seq:
   *   status.generate_fault(xml_doc) -> XmlDoc
   *
   */
  WsXmlDocH generate_fault(WsXmlDocH doc) {
    return wsman_generate_fault( doc, $self->fault_code, $self->fault_detail_code, $self->fault_msg);
  }
}

/*
 * WsManFault -> Fault
 */

%rename(Fault) _WsManFault;
%nodefault _WsManFault;
struct _WsManFault {};
typedef struct _WsManFault WsManFault;

/*
 * Document-class: Fault
 *
 * Fault represents details of a failed WS-Management operation
 *
 */
%extend _WsManFault {
  /*
   * Create a Fault representation of a failed WS-Management result doc
   *
   * call-seq:
   *   Openwsman::Fault.new result -> Openwsman::Fault
   *
   */
  _WsManFault(WsXmlDocH doc) {
    WsManFault *fault = wsmc_fault_new();
    wsmc_get_fault_data(doc, fault);
    return fault;
  }
  ~_WsManFault() {
    wsmc_fault_destroy($self);
  }
  /*
   * Fault code
   *
   * call-seq:
   *   fault.code -> String
   *
   */
  const char *code() {
    return $self->code;
  }
  /*
   * Fault subcode
   *
   * call-seq:
   *   fault.subcode -> String
   *
   */
  const char *subcode() {
    return $self->subcode;
  }
  /*
   * Fault reason
   *
   * call-seq:
   *   fault.reason -> String
   *
   */
  const char *reason() {
    return $self->reason;
  }
  /*
   * Fault detail
   *
   * call-seq:
   *   fault.detail -> String
   *
   */
  const char *detail() {
    return $self->fault_detail;
  }
}
