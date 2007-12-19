#if defined(SWIGPYTHON)
%module pywsman
#endif
#if defined(SWIGRUBY)
%module rbwsman
#endif

/* to be copied to the .c output file */
%{
#include "wsman-api.h"
#include "openwsman.h"
%}


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

%ignore WsXmlNode___undefined;
%ignore __WsXmlAttr___undefined;
%ignore __WsXmlNs___undefined;

%rename(WsXmlNode) __WsXmlNode;
%rename(WsXmlAttr) __WsXmlAttr;
%rename(WsXmlNs) __WsXmlNs;
%include "wsman-types.h"


/*-----------------------------------------------------------------*/
/* xml structure accessors */

%{
#include "wsman-xml-api.h"
%}

%extend __WsXmlNode {
  char *dump() {
    int size;
    char *buf;
    ws_xml_dump_memory_node_tree( $self, &buf, &size );
    return buf;
  }
  char *dump_enc(const char *encoding) {
    int size;
    char *buf;
    ws_xml_dump_memory_enc( $self, &buf, &size, encoding );
    return buf;
  }			      
}

/*-----------------------------------------------------------------*/
/* options */

client_opt_t *wsmc_options_init(void);
void wsmc_options_destroy(client_opt_t * op);

void wsmc_set_filter(const char *filter, client_opt_t * options);

void wsmc_set_dialect(const char *dialect, client_opt_t * options);
 
void _set_assoc_filter(client_opt_t *options);


/*-----------------------------------------------------------------*/
/* client */
WsManClient *wsmc_create_from_uri(const char *endpoint);

WsManClient *wsmc_create(const char *hostname,
                                         const int port, const char *path,
                                         const char *scheme,
                                         const char *username,
                                         const char *password);

void wsmc_release(WsManClient * cl);

void wsmc_set_dumpfile( WsManClient *cl, FILE *f );

void wsmc_add_selector(client_opt_t * options, const char *key, const char *value);

long wsmc_get_response_code(WsManClient * cl);

void wsmc_set_action_option(client_opt_t * options, unsigned int);


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