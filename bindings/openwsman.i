#if defined(SWIGJAVA)
%module jwsman
#endif

#if defined(SWIGPYTHON)
%module pywsman
#endif

#if defined(SWIGCSHARP)
%module cswsman
#endif

#if defined(SWIGRUBY)

%module rbwsman

%{
#include <rubyio.h>
#include <ruby.h>
%}

%typemap(in) FILE* {
  struct OpenFile *fptr;

  Check_Type($input, T_FILE);
  GetOpenFile($input, fptr);
  /*rb_io_check_writable(fptr);*/
  $1 = GetReadFile(fptr);
}

/*SWIG 1.3.33: %feature("autodoc","1")*/
#endif

#if defined(SWIGPERL)
%module openwsman

//==================================
// Typemap: Allow FILE* as PerlIO
//----------------------------------
%typemap(in) FILE* {
    $1 = PerlIO_findFILE(IoIFP(sv_2io($input)));
}
#endif

%{
#include <wsman-types.h>
#include <wsman-client.h>
#include <wsman-api.h>
#include <wsman-xml-api.h>
#include <wsman-epr.h>
#include <wsman-filter.h>
/*#include "openwsman.h"*/
%}

/*-----------------------------------------------------------------*/
/* type definitions */

/* local definitions
 *
 * Openwsman handles some structures as 'anonymous', just declaring
 * them without exposing their definition.
 * However, SWIG need the definition in order to create bindings.
 */


%rename(Client) _WsManClient;
%nodefault _WsManClient;
typedef struct _WsManClient {
} WsManClient;

%rename(EndPointReference) epr_t;
%nodefault epr_t;
typedef struct {
    char * address;
} epr_t;


%rename(Filter) filter_t;
%nodefault filter_t;
typedef struct {
    char *resultClass;
    char *assocClass;
} filter_t;

%rename(ClientOptions) client_opt_t;
%nodefault client_opt_t;
typedef struct {
} client_opt_t;

%nodefault __WsXmlNode; /* part of WsXmlDoc */
%rename(WsXmlNode) __WsXmlNode;
%ignore __WsXmlNode::__undefined;

%nodefault __WsXmlAttr; /* part of WsXmlNode */
%rename(WsXmlAttr) __WsXmlAttr;
%ignore __WsXmlAttr::__undefined;

%nodefault __WsXmlNs;   /* part of WsXmlAttr */
%rename(WsXmlNs) __WsXmlNs;
%ignore __WsXmlNs::__undefined;

%nodefault _WsXmlDoc;
%rename(WsXmlDoc) _WsXmlDoc;
struct _WsXmlDoc {};

%include "wsman-types.h"


/*-----------------------------------------------------------------*/
/* constant definitions */

%include "wsman-names.h"

#define FLAG_NONE                            0x0000
#define FLAG_ENUMERATION_COUNT_ESTIMATION    0x0001
#define FLAG_ENUMERATION_OPTIMIZATION        0x0002
#define FLAG_ENUMERATION_ENUM_EPR            0x0004
#define FLAG_ENUMERATION_ENUM_OBJ_AND_EPR    0x0008
#define FLAG_DUMP_REQUEST                    0x0010
#define FLAG_IncludeSubClassProperties       0x0020
#define FLAG_ExcludeSubClassProperties       0x0040
#define FLAG_POLYMORPHISM_NONE               0x0080
#define FLAG_MUND_MAX_ESIZE                  0x0100
#define FLAG_MUND_LOCALE                     0x0200
#define FLAG_MUND_OPTIONSET                  0x0400
#define FLAG_MUND_FRAGMENT                   0x0800
#define FLAG_CIM_EXTENSIONS                  0x1000
#define FLAG_CIM_REFERENCES                  0x2000
#define FLAG_CIM_ASSOCIATORS                 0x4000
#define FLAG_EVENT_SENDBOOKMARK		     0X8000


#define	WSMAN_DELIVERY_PUSH         0
#define WSMAN_DELIVERY_PUSHWITHACK  1
#define WSMAN_DELIVERY_EVENTS       2
#define WSMAN_DELIVERY_PULL         3


/*-----------------------------------------------------------------*/
/* xml structure accessors */

%extend _WsXmlDoc {
  /* constructor */
  _WsXmlDoc() {
    return ws_xml_create_soap_envelope();
  }
  /* destructor */
  ~_WsXmlDoc() {
    ws_xml_destroy_doc( $self );
  }
#if defined(SWIGRUBY)
  %alias dump "to_s";
#endif
  /* dump doc as string */
  char *dump(const char *encoding="utf-8") {
    int size;
    char *buf;
    ws_xml_dump_memory_enc( $self, &buf, &size, encoding );
    return buf;
  }
  /* dump doc to file */
  void dump_file(FILE *fp) {
    ws_xml_dump_doc( fp, $self );
  }			      
  /* get root node of doc */
  WsXmlNodeH root() {
    return ws_xml_get_doc_root( $self );
  }
  /* get soap envelope */
  WsXmlNodeH envelope() {
    return ws_xml_get_soap_envelope( $self );
  }
  /* get soap header */
  WsXmlNodeH header() {
    return ws_xml_get_soap_header( $self );
  }
  /* get soap body */
  WsXmlNodeH body() {
    return ws_xml_get_soap_body( $self );
  }
  /* get soap element by name */
  WsXmlNodeH element(const char *name) {
    return ws_xml_get_soap_element( $self, name );
  }
}


%extend __WsXmlNode {
#if defined(SWIGRUBY)
  %alias text "to_s";
#endif
  /* dump node as string */
  char *dump() {
    int size;
    char *buf;
    ws_xml_dump_memory_node_tree( $self, &buf, &size );
    return buf;
  }
  /* dump node to file */
  void dump_file(FILE *fp) {
    ws_xml_dump_node_tree( fp, $self );
  }
  /* get text of node */
  char *text() {
    return ws_xml_get_node_text( $self );
  }
#if defined(SWIGRUBY)
  %rename( "text=" ) set_text( const char *text );
#endif
  void set_text( const char *text ) {
    ws_xml_set_node_text( $self, text );
  }
  /* get doc for node */
  WsXmlDocH doc() {
    return ws_xml_get_node_doc( $self );
  }
  /* get parent for node */
  WsXmlNodeH parent() {
    return ws_xml_get_node_parent( $self );
  }
  /* get name for node */
  char *name() {
    return ws_xml_get_node_local_name( $self );
  }
  /* set name of node */
  void set_name( const char *nsuri, const char *name ) {
    ws_xml_set_node_name( $self, nsuri, name );
  }
  /* get namespace for node */
  char *ns() {
    return ws_xml_get_node_name_ns( $self );
  }
  /* set namespace of node */
  void set_ns( const char *ns, const char *prefix ) {
    ws_xml_set_ns( $self, ns, prefix );
  }
  
  /* find node within tree */
  WsXmlNodeH find( const char *ns, const char *name, int recursive = 1) {
    return ws_xml_find_in_tree( $self, ns, name, recursive );
  }
				 
  /* count node children */
  int child_count() {
    return ws_xml_get_child_count( $self );
  }
  /* add child to node */
  WsXmlNodeH child_add( const char *ns, const char *name, const char *value = NULL ) {
    return ws_xml_add_child( $self, ns, name, value );
  }
#if defined(SWIGRUBY)
  /* enumerate children */
  void each_child() {
    int i = 0;
    while ( i < ws_xml_get_child_count( $self ) ) {
      rb_yield( SWIG_NewPointerObj((void*) ws_xml_get_child($self, i, NULL, NULL), SWIGTYPE_p___WsXmlNode, 0));
      ++i;
    }
  }
#endif
  
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


/*-----------------------------------------------------------------*/
/* epr */
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

/*-----------------------------------------------------------------*/
/* filter */
%extend filter_t {
    filter_t() {
        return filter_initialize();
    }
  ~filter_t() {
    filter_destroy( $self );
  }

  int associators( epr_t *epr, const char *assocClass, const char *resultClass,
        const char *role, const char *resultRole, char **resultProp, const int propNum) {
    return filter_set_assoc($self, epr, 0, assocClass, resultClass, role, resultRole, resultProp, propNum);
  }
  int references( epr_t *epr, const char *assocClass,
    const char *resultClass, const char *role, const char *resultRole, char **resultProp, const int propNum) {
    return filter_set_assoc($self, epr, 1, assocClass, resultClass, role, resultRole, resultProp, propNum);
  }

  int simple(const char *dialect, const char *query) {
    return filter_set_simple($self, dialect, query );
  }
  int xpath(const char *query) {
    return filter_set_simple($self, WSM_XPATH_FILTER_DIALECT, query );
  }
  int cql(const char *query) {
    return filter_set_simple($self, WSM_CQL_FILTER_DIALECT, query );
  }
  int wql(const char *query) {
    return filter_set_simple($self, WSM_WQL_FILTER_DIALECT, query );
  }

}

/*-----------------------------------------------------------------*/
/* options */

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


#if defined(SWIGRUBY)
/*
 * call-seq:
 *   WsMan::debug
 *
 * Return openwsman debug level.
 */

static VALUE
rwsman_debug_get( VALUE module )
{
    debug_level_e d = wsman_debug_get_level();
    return INT2FIX( d );
}


/*
 * call-seq:
 *   WsMan::debug = 1
 *   WsMan::debug = 0
 *
 * Set openwsman debug level.
 */

static VALUE
rwsman_debug_set( VALUE module, VALUE dbg )
{
    static int init = 0;
    int d = FIX2INT( dbg );

    if (!init && d != 0) {
	init = 1;
	debug_add_handler( debug_message_handler, DEBUG_LEVEL_ALWAYS, NULL );
    }
    wsman_debug_set_level( d );

    return Qnil;
}
#endif

/*-----------------------------------------------------------------*/
/* client */

%extend WsManClient {
  /* constructor */
  WsManClient( const char *uri ) {
    return wsmc_create_from_uri( uri );
  }
  WsManClient(const char *hostname,
              const int port, const char *path,
              const char *scheme,
              const char *username,
              const char *password) {
    return wsmc_create( hostname, port, path, scheme, username, password );
  }
  /* destructor */
  ~WsManClient() {
    wsmc_release( $self );
  }
  /* set dumpfile */
#if defined(SWIGRUBY)
  %rename( "dumpfile=" ) set_dumpfile( FILE *f );
#endif
  void set_dumpfile( FILE *f ) {
    wsmc_set_dumpfile( $self, f );
  }
  long response_code() {
    return wsmc_get_response_code( $self );
  }
  char *scheme() {
    return wsmc_get_scheme( $self );
  }
  char *host() {
    return wsmc_get_hostname( $self );
  }
  int port() {
    return wsmc_get_port( $self );
  }
  char *path() {
    return wsmc_get_path( $self );
  }
  char *user() {
    return wsmc_get_user( $self );
  }
  char *password() {
    return wsmc_get_password( $self );
  }
}


/*-----------------------------------------------------------------*/
/* actions */

%extend WsManClient {
  WsXmlDocH identify( client_opt_t *options ) {
    return wsmc_action_identify( $self, options );
  }
  
  WsXmlDocH get_from_epr( client_opt_t *options , epr_t *epr) {
    return wsmc_action_get_from_epr( $self, epr, options);
  }
  WsXmlDocH delete_from_epr( client_opt_t *options , epr_t *epr) {
    return wsmc_action_delete_from_epr( $self, epr, options);
  }

  WsXmlDocH enumerate( client_opt_t *options , filter_t *filter, char *resource_uri) {
    return wsmc_action_enumerate( $self, resource_uri, options, filter);
  }
  WsXmlDocH pull( client_opt_t *options , filter_t *filter, char *resource_uri, char *enum_ctx) {
    return wsmc_action_pull( $self, resource_uri, options, filter, enum_ctx);
  }
  WsXmlDocH create( client_opt_t *options, char *resource_uri, char *data, size_t size, char *encoding) {
    return wsmc_action_create_fromtext( $self, resource_uri, options, data, size, encoding);
  }
  WsXmlDocH put( client_opt_t *options , char *resource_uri,  char *data, size_t size, char *encoding) {
    return wsmc_action_put_fromtext( $self, resource_uri, options, data, size, encoding);
  }
  WsXmlDocH release( client_opt_t *options , char *resource_uri, char *enum_ctx) {
    return wsmc_action_release( $self, resource_uri, options, enum_ctx);
  }
  WsXmlDocH get( client_opt_t *options , char *resource_uri) {
    return wsmc_action_get( $self, resource_uri, options);
  }
  WsXmlDocH delete( client_opt_t *options , char *resource_uri) {
    return wsmc_action_delete( $self, resource_uri, options);
  }
  WsXmlDocH invoke( client_opt_t *options , char *resource_uri, char *method, char *data, size_t size, char *encoding) {
    return wsmc_action_invoke_fromtext( $self, resource_uri, options, method, data, size, encoding);
  }

  WsXmlDocH subscribe(client_opt_t *options , filter_t *filter, char *resource_uri) {
    return wsmc_action_subscribe($self,  resource_uri, options, filter);
  }

  WsXmlDocH unsubscribe(client_opt_t *options , filter_t *filter, char *resource_uri, char *identifier) {
    return wsmc_action_unsubscribe($self, resource_uri, options, identifier);
  }

  WsXmlDocH renew(client_opt_t *options , char *resource_uri, char *identifier) {
    return wsmc_action_renew($self, resource_uri, options, identifier);
  }

}



