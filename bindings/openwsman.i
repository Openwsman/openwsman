#if defined(SWIGPYTHON)
%module pywsman
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

/* to be copied to the .c output file */
%{
#include "wsman-api.h"
#include "openwsman.h"
%}

/*-----------------------------------------------------------------*/
/* local definitions
 *
 * Openwsman handles some structures as 'anonymous', just declaring
 * them without exposing their definition.
 * However, SWIG need the definition in order to create bindings.
 */
 
%rename(Client) _WsManClient;
%nodefault _WsManClient;

%rename(WsXmlDoc) _WsXmlDoc;
%nodefault _WsXmlDoc;

%rename(ClientOptions) client_opt_t;
%nodefault client_opt_t;

struct _WsXmlDoc {
};

struct _WsManClient {
};

typedef struct {
} client_opt_t;


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
#define FLAG_EVENT_SENDBOOKMARK		0X8000


#define	WSMAN_DELIVERY_PUSH  0
#define WSMAN_DELIVERY_PUSHWITHACK  1
#define WSMAN_DELIVERY_EVENTS 2
#define WSMAN_DELIVERY_PULL 3


/*-----------------------------------------------------------------*/
/* type definitions */

%nodefault __WsXmlNode; /* part of WsXmlDoc */
%nodefault __WsXmlAttr; /* part of WsXmlNode */
%nodefault __WsXmlNs;   /* part of WsXmlAttr */

%rename(WsXmlNode) __WsXmlNode;
%rename(WsXmlAttr) __WsXmlAttr;
%rename(WsXmlNs) __WsXmlNs;

%ignore __WsXmlNode::__undefined;
%ignore __WsXmlAttr::__undefined;
%ignore __WsXmlNs::__undefined;

%include "wsman-types.h"


/*-----------------------------------------------------------------*/
/* xml structure accessors */

%{
#include "wsman-xml-api.h"
%}

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
  /* get namespace for node */
  char *ns() {
    return ws_xml_get_node_name_ns( $self );
  }
  /* find node within tree */
  WsXmlNodeH find( const char *ns, const char *name, int recursive = 1) {
    return ws_xml_find_in_tree( $self, ns, name, recursive );
  }
				 
  /* count node children */
  int child_count() {
    return ws_xml_get_child_count( $self );
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
/* options */

%extend client_opt_t {
  client_opt_t() {
    return wsmc_options_init();
  }
  ~client_opt_t() {
    wsmc_options_destroy( $self );
  }

  /* set filter */
#if defined(SWIGRUBY)
  %rename( "filter=" ) set_filter( const char *filter );
#endif
  void set_filter( const char *filter ) {
    wsmc_set_filter( filter, $self );
  }
  
  /* set dialect  */
#if defined(SWIGRUBY)
  %rename( "dialect=" ) set_dialect( const char *dialect );
#endif
  void set_dialect( const char *dialect ) {
    wsmc_set_dialect( dialect, $self );
  }

  void set_assoc_filter(void) {
   wsmc_set_action_option($self, FLAG_CIM_ASSOCIATORS);
  }

  void set_dump_request(void) {
    wsmc_set_action_option($self, FLAG_DUMP_REQUEST );
  }

  void add_selector(char *key, char*value) {
    wsmc_add_selector($self, key, value);
  }
}

void wsmc_set_action_option(client_opt_t * options, unsigned int);

/*-----------------------------------------------------------------*/
/* client */

%extend _WsManClient {
  /* constructor */
  _WsManClient( const char *uri ) {
    return wsmc_create_from_uri( uri );
  }
  _WsManClient(const char *hostname,
              const int port, const char *path,
              const char *scheme,
              const char *username,
              const char *password) {
    return wsmc_create( hostname, port, path, scheme, username, password );
  }
  /* destructor */
  ~_WsManClient() {
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

void wsmc_add_selector(client_opt_t * options, const char *key, const char *value);



/*-----------------------------------------------------------------*/
/* actions */

%extend _WsManClient {
  WsXmlDocH identify( client_opt_t *options ) {
    return wsmc_action_identify( $self, options );
  }
  WsXmlDocH enumerate( client_opt_t *options , char *resource_uri) {
    return wsmc_action_enumerate( $self, resource_uri, options);
  }
  WsXmlDocH pull( client_opt_t *options , char *resource_uri, char *enum_ctx) {
    return wsmc_action_pull( $self, resource_uri, options, enum_ctx);
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
}

char *_identify(WsManClient * cl, client_opt_t * options, char *encoding);

char *_pull(WsManClient * cl, const char *resource_uri, 
	client_opt_t * options, const char *enumContext, char *encoding);
	
char *_release(WsManClient * cl, const char *resource_uri, client_opt_t * options, 
	const char *enumContext, char *encoding);

char *_enumerate(WsManClient * cl, const char *resource_uri, client_opt_t * options,  char *encoding);

char *_get(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);

char *_delete(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);

char *_invoke(WsManClient * cl, const char *resource_uri, client_opt_t * options, 
	const char *method, const char *data, size_t size, char *encoding);

char *_put(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *data, size_t size, char *encoding);

char *_subscribe(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding);

char *_renew(WsManClient *cl, const char *resource_uri, client_opt_t *options, char *uuid, char *encoding);

char *_unsubscribe(WsManClient *cl, const char *resource_uri, client_opt_t *op, char *uuid, char *encoding);


	void
	wsmc_set_delivery_uri(const char *delivery_uri, client_opt_t * options);
	

	void
	wsmc_set_sub_expiry(int event_subscription_expire, client_opt_t * options);
	

	void
	wsmc_set_heartbeat_interval(int heartbeat_interval, client_opt_t * options);
	

	void
	wsmc_set_delivery_mode(WsmanDeliveryMode delivery_mode, client_opt_t * options);
