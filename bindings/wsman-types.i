/*
 * wsman-types.i
 * type definitions for openwsman swig bindings
 */

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

%nodefault _WS_CONTEXT;
%rename(WsContext) _WS_CONTEXT;
struct _WS_CONTEXT {};

%include "wsman-types.h"
