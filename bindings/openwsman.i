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
 * local-defs.h provides dummy definitions so the classes are available
 * in SWIG
 */
 
%rename(Client) _WsManClient;
%nodefault _WsManClient;

%rename(WsXmlDoc) _WsXmlDoc;
%nodefault _WsXmlDoc;

%include "local-defs.h"


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

%ignore __WsXmlNode___undefined;
%ignore __WsXmlAttr___undefined;
%ignore __WsXmlNs___undefined;

%include "wsman-types.h"


/*-----------------------------------------------------------------*/
/* xml structure accessors */

%{
#include "wsman-xml-api.h"
%}

%extend __WsXmlNode {
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
}

%extend _WsXmlDoc {
  /* destructor */
  ~_WsXmlDoc() {
    ws_xml_destroy_doc( $self );
  }
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

%rename(create_soap_envelope) ws_xml_create_soap_envelope;

/*-----------------------------------------------------------------*/
/* options */

%nodefault client_opt_t;
%rename(ClientOptions) client_opt_t;
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
    wsmc_set_dialect( filter, $self );
  }
}

void _set_assoc_filter(client_opt_t *options);
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