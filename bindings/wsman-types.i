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

/*
 * hash_t typemaps
 */
%typemap(out) hash_t * {
 $result = hash2value($1);
}

%typemap(in) hash_t * {
 $input = value2hash(NULL, $1);
}

/*
 * Client
 */

%rename("Client") _WsManClient;
%nodefault _WsManClient;
typedef struct _WsManClient {
} WsManClient;

/*
 * Transport
 *  this is just a convenience alias for WsManClient
 *  see wsman-transport.i
 */
 
%rename("Transport") _WsManTransport;
%nodefault _WsManTransport;
typedef struct _WsManTransport {
} WsManTransport;

/*
 * EndPointReference
 */
 
%rename("EndPointReference") epr_t;
%nodefault epr_t;
typedef struct {
    char * address;
} epr_t;

/*
 * Filter
 */
 
%rename("Filter") filter_t;
%nodefault filter_t;
typedef struct {
    char *resultClass;
    char *assocClass;
} filter_t;

/*
 * ClientOptions
 */
 
%rename("ClientOptions") client_opt_t;
%nodefault client_opt_t;
typedef struct {
} client_opt_t;

/*
 * WsXmlNode
 */
 
%nodefault __WsXmlNode; /* part of WsXmlDoc */
%rename("XmlNode") __WsXmlNode;
%ignore __WsXmlNode::__undefined;

/*
 * WsXmlAttr
 */
 
%nodefault __WsXmlAttr; /* part of WsXmlNode */
%rename("XmlAttr") __WsXmlAttr;
%ignore __WsXmlAttr::__undefined;

/*
 * WsXmlNs
 */
 
%nodefault __WsXmlNs;   /* part of WsXmlAttr */
%rename("XmlNs") __WsXmlNs;
%ignore __WsXmlNs::__undefined;

/*
 * WsXmlDoc
 */
 
%nodefault _WsXmlDoc;
%rename("XmlDoc") _WsXmlDoc;
struct _WsXmlDoc {};

/*
 * WsContext
 */
 
%nodefault _WS_CONTEXT;
%rename("Context") _WS_CONTEXT;
struct _WS_CONTEXT {};

/*
 * WsEnumerateInfo
 */
 
%nodefault __WsEnumerateInfo;
%rename("EnumerateInfo") __WsEnumerateInfo;
struct __WsEnumerateInfo {};

/*
 * Soap
 */
 
%nodefault __Soap;
%rename("Soap") __Soap;
struct __Soap {};

/*
 * SoapOp
 */
 
%nodefault __SoapOp;
%rename("SoapOp") __SoapOp;
struct __SoapOp {};

/*
 * WsmanStatus
 */
 
%nodefault _WsmanStatus;
%rename("Status") _WsmanStatus;
struct _WsmanStatus {};

%include "wsman-types.h"
