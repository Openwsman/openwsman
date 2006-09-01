/*******************************************************************************
* Copyright (C) 2004-2006 Intel Corp. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
*  - Neither the name of Intel Corp. nor the names of its
*    contributors may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 * @author Anas Nashif
 * @author Eugene Yarmosh
 */

#ifndef SOAP_API_H_
#define SOAP_API_H_


#define PROCESSED_MSG_ID_MAX_SIZE	        200
#define WSMAN_MINIMAL_ENVELOPE_SIZE_REQUEST     8192

#define DMTF_WSMAN_SPEC_1

#define SOAP1_2_CONTENT_TYPE            "application/soap+xml; charset=utf-8"
#define SOAP_CONTENT_TYPE               "application/soap+xml"

#define SOAP_SKIP_DEF_FILTERS		0x01
#define SOAP_ACTION_PREFIX		0x02 // otherwise exact

#define SOAP_ONE_WAY_OP			0x04
#define SOAP_NO_RESP_OP			0x08
#define SOAP_DONT_KEEP_INDOC		0x10

#define SOAP_CLIENT_RESPONSE		0x20 // internal use
#define SOAP_CUSTOM_DISPATCHER		0x40 // internal use


#define XML_NS_SOAP_1_1                 "http://schemas.xmlsoap.org/soap/envelope"
#define XML_NS_SOAP_1_2                 "http://www.w3.org/2003/05/soap-envelope"


#define XML_NS_XML_NAMESPACES	        "http://www.w3.org/XML/1998/namespace"	
#define XML_NS_ADDRESSING	        "http://schemas.xmlsoap.org/ws/2004/08/addressing"	
#define XML_NS_DISCOVERY		"http://schemas.xmlsoap.org/ws/2004/10/discovery"	
#define XML_NS_EVENTING		        "http://schemas.xmlsoap.org/ws/2004/08/eventing"	
#define XML_NS_ENUMERATION	        "http://schemas.xmlsoap.org/ws/2004/09/enumeration"
#define XML_NS_TRANSFER		        "http://schemas.xmlsoap.org/ws/2004/09/transfer"
#define XML_NS_XML_SCHEMA	        "http://www.w3.org/2001/XMLSchema"
#define XML_NS_SCHEMA_INSTANCE	        "http://www.w3.org/2001/XMLSchema-instance"

// CIM
#ifdef DMTF_WSMAN_SPEC_1
#define XML_NS_CIM_SCHEMA	        "http://schemas.dmtf.org/wbem/wscim/1/common"
#define XML_NS_CIM_CLASS                "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2"
#define XML_NS_CIM_BINDING              "http://schemas.dmtf.org/wbem/wsman/1/cimbinding.xsd"

#else // DMTF_WSMAN_SPEC_1
#define XML_NS_CIM_SCHEMA               "http://schemas.dmtf.org/cimv2.9/CIM_Schema"
#define XML_NS_CIM_CLASS		"http://schemas.dmtf.org/wsman/2005/06/cimv2.9"
#endif


// WS-Management

#ifdef DMTF_WSMAN_SPEC_1
#define XML_NS_WS_MAN                   "http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd"
#else
#define XML_NS_WS_MAN			"http://schemas.xmlsoap.org/ws/2005/06/management"
#endif


#define XML_NS_WS_MAN_CAT	        "http://schemas.xmlsoap.org/ws/2005/06/wsmancat"

#define XML_NS_WSMAN_ID                 "http://schemas.dmtf.org/wbem/wsman/identity/1/wsmanidentity.xsd"
// http://schemas.dmtf.org/wbem/wsman/identify/1/wsmanidentity.xsd



#define WSMID_IDENTIFY                  "Identify"
#define WSMID_IDENTIFY_RESPONSE         "IdentifyResponse"
#define WSMID_PROTOCOL_VERSION          "ProtocolVersion"
#define WSMID_PRODUCT_VENDOR            "ProductVendor"
#define WSMID_PRODUCT_VERSION           "ProductVersion"


#define XML_SCHEMA_NIL			"nil"


#define WSA_TO_ANONYMOUS\
                "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous"
#define WSA_MESSAGE_ID		        "MessageID"
#define WSA_ADDRESS			"Address"
#define WSA_EPR				"EndpointReference"
#define WSA_ACTION			"Action"
#define WSA_RELATES_TO		        "RelatesTo"
#define WSA_TO				"To"
#define WSA_REPLY_TO		        "ReplyTo"
#define WSA_FAULT_TO			"FaultTo"
#define WSA_REFERENCE_PROPERTIES	"ReferenceProperties"
#define WSA_REFERENCE_PARAMETERS	"ReferenceParameters"
#define WSA_ACTION_FAULT\
		"http://schemas.xmlsoap.org/ws/2004/08/addressing/fault"



#define SOAP_ENVELOPE			"Envelope"
#define SOAP_HEADER			"Header"
#define SOAP_BODY			"Body"
#define SOAP_FAULT			"Fault"
#define SOAP_CODE			"Code"
#define SOAP_VALUE			"Value"
#define SOAP_SUBCODE			"Subcode"
#define SOAP_REASON			"Reason"
#define SOAP_TEXT			"Text"
#define SOAP_LANG			"lang"
#define SOAP_DETAIL			"Detail"
#define SOAP_FAULT_DETAIL		"FaultDetail"
#define SOAP_MUST_UNDERSTAND		"mustUnderstand"
#define SOAP_VERSION_MISMATCH	        "VersionMismatch"
#define SOAP_UPGRADE			"Upgrade"
#define SOAP_SUPPORTED_ENVELOPE	        "SupportedEnvelope"


#define TRANSFER_ACTION_PUT		"http://schemas.xmlsoap.org/ws/2004/09/transfer/Put"
#define TRANSFER_ACTION_GET		"http://schemas.xmlsoap.org/ws/2004/09/transfer/Get"
#define TRANSFER_GET			"Get"
#define TRANSFER_PUT			"Put"

#define ENUM_ACTION_ENUMERATE	        "http://schemas.xmlsoap.org/ws/2004/09/enumeration/Enumerate"
#define ENUM_ACTION_RELEASE		"http://schemas.xmlsoap.org/ws/2004/09/enumeration/Release"
#define ENUM_ACTION_PULL		"http://schemas.xmlsoap.org/ws/2004/09/enumeration/Pull"
#define WSENUM_ENUMERATE		"Enumerate"
#define WSENUM_ENUMERATE_RESP	        "EnumerateResponse"
#define WSENUM_PULL			"Pull"
#define WSENUM_PULL_RESP		"PullResponse"
#define WSENUM_END_TO			"EndTo"
#define WSENUM_EXPIRES			"Expires"
#define WSENUM_FILTER			"Filter"
#define WSENUM_DIALECT			"Dialect"
#define WSENUM_ENUMERATION_CONTEXT	"EnumerationContext"
#define WSENUM_MAX_TIME			"MaxTime"
#define WSENUM_MAX_ELEMENTS		"MaxElements"
#define WSENUM_MAX_CHARACTERS	        "MaxCharacters"
#define WSENUM_ITEMS			"Items"
#define WSENUM_END_OF_SEQUENCE	        "EndOfSequence"
#define WSENUM_RELEASE			"Release"
#define WSENUM_RELEASE_RESP		"ReleaseResponse"
#define WSENUM_RENEW			"Renew"
#define WSENUM_RENEW_RESP		"RenewResponse"
#define WSENUM_GET_STATUS		"GetStatus"
#define WSENUM_GET_STATUS_RESP	        "GetStatusResponse"
#define WSENUM_ENUMERATION_END	        "EnumerationEnd"
#define WSENUM_REASON			"Reason"
#define WSENUM_CODE			"Code"
#define WSENUM_SOURCE_SHUTTING_DOWN	"SourceShuttingDown"
#define WSENUM_SOURCE_CANCELING	        "SourceCanceling"


#define WSM_SYSTEM			"System"
#define WSM_RESOURCE_URI		"ResourceURI"
#define WSM_SELECTOR_SET		"SelectorSet"
#define WSM_SELECTOR			"Selector"
#define WSM_NAME		        "Name"
#define WSM_REQUEST_TOTAL		"RequestTotalItemsCountEstimate"
#define WSM_TOTAL_ESTIMATE              "TotalItemsCountEstimate"
#define WSM_OPTIMIZE_ENUM               "OptimizeEnumeration"
#define WSM_MAX_ELEMENTS                "MaxElements"
#define WSM_ENUM_EPR                    "EnumerateEPR"
#define WSM_ENUM_OBJ_AND_EPR            "EnumerateObjectAndEPR"
#define WSM_ENUM_MODE                   "EnumerationMode"
#define WSM_ITEM                        "Item"
#define WSM_FRAGMENT_TRANSFER           "FragmentTransfer"
#define WSM_XML_FRAGMENT                "XmlFragment"

#define WSM_MAX_ENVELOPE_SIZE	        "MaxEnvelopeSize"
#define WSM_OPERATION_TIMEOUT	        "OperationTimeout"
#define WSM_FAULT_SUBCODE	        "FaultSubCode"

// Catalog

#define WSMANCAT_RESOURCE		"Resource"
#define WSMANCAT_RESOURCE_URI	        "ResourceUri"
#define WSMANCAT_VERSION		"Version"
#define WSMANCAT_NOTES			"Notes"
#define WSMANCAT_VENDOR			"Vendor"
#define WSMANCAT_DISPLAY_NAME	        "DisplayName"
#define WSMANCAT_KEYWORDS		"Keywords"
#define WSMANCAT_ACCESS			"Access"
#define WSMANCAT_RELATIONSHIPS	        "Relationsships"
#define WSMANCAT_COMPLIANCE		"Compliance"
#define WSMANCAT_OPERATION		"Operation"
#define WSMANCAT_SELECTOR_SET	        "SelectorSet"
#define WSMANCAT_SELECTOR		"Selector"
#define WSMANCAT_OPTION_SET		"OptionSet"
#define WSMANCAT_ACTION			"Action"
#define WSMANCAT_SELECTOR_SET_REF       "SelectorSetRef"
#define WSMANCAT_LOCATION		"Location"
#define WSMANCAT_NAME			"Name"
#define WSMANCAT_TYPE			"Type"


// Filter Dialects
#define WSM_SELECTOR_FILTER_DIALECT     "http://schemas.dmtf.org/wbem/wsman/1/wsman/SelectorFilter"
#define WSM_WQL_FILTER_DIALECT          "http://schemas.microsoft.com/wbem/wsman/1/WQL"
#define WSM_XPATH_FILTER_DIALECT        "http://www.w3.org/TR/1999/REC-xpath-19991116"


#define WSFW_RESPONSE_STR		"Response"
#define WSFW_INDOC			"indoc"

#define WSFW_ENUM_PREFIX		"_en."

#define SOAP_MAX_RESENT_COUNT		10

// context

#define WS_CONTEXT_TYPE_MASK		0x0f
#define WS_CONTEXT_FREE_DATA		0x80

#define WS_CONTEXT_TYPE_STRING		0x01
#define WS_CONTEXT_TYPE_ULONG		0x02
#define WS_CONTEXT_TYPE_XMLDOC		0x03
#define WS_CONTEXT_TYPE_XMLNODE		0x04
#define WS_CONTEXT_TYPE_BLOB		0x05
#define WS_CONTEXT_TYPE_FAULT		0x06

#include "wsman-faults.h"
struct _WsmanStatus {
    WsmanFaultCodeType fault_code;
    WsmanFaultDetailType fault_detail_code;
    char *fault_msg;
};
typedef struct _WsmanStatus WsmanStatus;

#include "wsman-soap-message.h"


struct __WsContext
{
	int __unk;
};
typedef struct __WsContext* WsContextH;

struct __SoapDispatch
{
	unsigned __undefined;
};
typedef struct __SoapDispatch* SoapDispatchH;

typedef SoapDispatchH (*DispatcherCallback)(WsContextH, void*, WsXmlDocH);

// this structure is similarly defined in wsman-xml-api.h
#ifndef _SoapH_Defined
#define _SoapH_Defined
struct __Soap
{
	unsigned __undefined;
};
typedef struct __Soap* SoapH;
#endif

struct __SoapOp
{
	unsigned __undefined;
};
typedef struct __SoapOp* SoapOpH;


struct __DispatchResponse
{
	char *buf;
	int httpCode;
};
typedef struct __DispatchResponse DispatchResponse;


typedef int (*SoapServiceCallback)(SoapOpH, void*);

struct __SOAP_FW
{
        pthread_mutex_t lockData;  // do not move this field
	int bExit;
	void* parserData;
	unsigned long uniqueIdCounter;
	
	DL_List inboundFilterList;
	DL_List outboundFilterList;
	
	DL_List dispatchList;	
	DL_List responseList;
	DL_List processedMsgIdList;

	WsContextH cntx; // TBD claen up and initilaize it;

	unsigned long lastResponseListScanTicks;
	
	// TBD: ??? Make it thread and pass as parameters
	int resendCount;
	unsigned long resentTimeout[SOAP_MAX_RESENT_COUNT];

	DL_List WsSerializerAllocList;
		
	void* dispatcherData;
	DispatcherCallback dispatcherProc;
};
typedef struct __SOAP_FW SOAP_FW;


struct _WS_CONTEXT_ENTRY
{
	DL_Node node;
	unsigned long size;
	unsigned long options;
	char* name;
};
typedef struct _WS_CONTEXT_ENTRY WS_CONTEXT_ENTRY;

struct _WS_CONTEXT
{
	SoapH soap;
	DL_List entries;
	int bUserCreatedCtnx; // to prevent user from destroying cntx he hasn't created
	// the fields below are for optimization
	WS_CONTEXT_ENTRY* lastEntry;
	int lastGetNameIndex;
};
typedef struct _WS_CONTEXT WS_CONTEXT;


struct __NameAliase
{
        char* name;
        char* aliase;
};
typedef struct __NameAliase NameAliase;
NameAliase* g_NameNameAliaseTable;

typedef void (*WsProcType)(void);




struct __PropertyInfo 
{
	char *key;
	char *val;
};
typedef struct __WsPropertyInfo WsPropertyInfo;

struct __WsSelectorInfo 
{
	char *key;
        char *type;
	char *val;
};
typedef struct __WsSelectorInfo WsSelectorInfo;

struct __WsSelector 
{
	char *name;
	char *value;
	char *type;
	char *description;	
};
typedef struct __WsSelector WsSelector;



struct __WsDispatchEndPointInfo
{
	unsigned long flags; // put/get/create/delete rpc enumerate/release/pull/update/getstatus	
	char* rqstName;
	char* respName;
	char* inAction;
	char* outAction;
	struct __XmlSerializerInfo* serializationInfo;
	WsProcType serviceEndPoint;
	void* data;
	struct __WsSelector* selectors;
};
typedef struct __WsDispatchEndPointInfo WsDispatchEndPointInfo;

struct __WsSupportedNamespaces
{
	char* ns;
};
typedef struct __WsSupportedNamespaces WsSupportedNamespaces;


struct __WsDispatchInterfaceInfo
{
	unsigned long flags;
	char* version;
	char* notes;
	char* vendor;
	char* displayName;
	char* compliance;	
	char* actionUriBase;
	char* wsmanResourceUri;
	void* extraData;
	WsSupportedNamespaces* namespaces;	
	WsDispatchEndPointInfo* endPoints;	
};
typedef struct __WsDispatchInterfaceInfo WsDispatchInterfaceInfo;


struct __DispatchToEpMap
{
	SoapDispatchH disp;
	WsDispatchEndPointInfo* ep;
	// WsDispatchInterfaceInfo* wsInterface;
};
typedef struct __DispatchToEpMap DispatchToEpMap;

struct __WsManDispatcherInfo
{
	int interfaceCount;
	int mapCount;
	GList *interfaces;
	DispatchToEpMap map[1];
};
typedef struct __WsManDispatcherInfo WsManDispatcherInfo;


struct __WsEnumerateInfo
{
//      char* enumerationId;
//      WsDispatchEndPointInfo* ep;
        unsigned long timeStamp;
        unsigned long timeout;
        unsigned int totalItems;
        unsigned int maxItems;
        unsigned char flags;
        int index;
        void* enumResults;
        void* pullResultPtr;
        void* appEnumContext;
};
typedef struct __WsEnumerateInfo WsEnumerateInfo;

typedef int (*WsEndPointEnumerate)(WsContextH, WsEnumerateInfo*, WsmanStatus*);
typedef int (*WsEndPointPull)(WsContextH, WsEnumerateInfo*, WsmanStatus*);
typedef int (*WsEndPointRelease)(WsContextH, WsEnumerateInfo*, WsmanStatus*);
typedef int (*WsEndPointPut)(WsContextH, void*, void**, WsmanStatus*);
typedef void* (*WsEndPointGet)(WsContextH, WsmanStatus*);

#define FLAG_ENUMERATION_COUNT_ESTIMATION    1
#define FLAG_ENUMERATION_OPTIMIZATION        2
#define FLAG_ENUMERATION_ENUM_EPR            4
#define FLAG_ENUMERATION_ENUM_OBJ_AND_EPR    8
#define FLAG_DUMP_REQUEST                    16

enum __WsmanFilterDialect
{
    WSMAN_FILTER_XPATH,
    WSMAN_FILTER_SELECTOR
};
typedef enum __WsmanFilterDialect WsmanFilterDialect;


struct _actionOptions {
    unsigned char       flags;
    char *              filter;
    char *              dialect;
    char *              fragment;
    char *              cim_ns;
    unsigned int        timeout;
    unsigned int        max_envelope_size;
};
typedef struct _actionOptions actionOptions;


/** *********************************** */

WsXmlDocH ws_get_context_xml_doc_val(WsContextH cntx, char* name);
char* wsman_get_selector(WsContextH cntx, WsXmlDocH doc, char* name, int index);

GList * wsman_get_selector_list(WsContextH cntx, WsXmlDocH doc);
        
        
void wsman_add_selector( WsXmlNodeH baseNode, char* name, char* val);
char* ws_addressing_get_action(WsContextH cntx, WsXmlDocH doc);
//void wsman_set_selector( WsXmlDocH doc, char* name, char* val);
char* wsman_get_system_uri(WsContextH cntx, WsXmlDocH doc);
char* wsman_get_resource_uri(WsContextH cntx, WsXmlDocH doc);

WS_CONTEXT_ENTRY* find_context_entry(WS_CONTEXT_ENTRY* start, char* name, int bPrefix);

void ws_initialize_context(WsContextH hCntx, SoapH soap);
WsContextH ws_create_context(SoapH soap);
SoapH ws_soap_initialize(void);
SoapH ws_context_get_runtime(WsContextH hCntx);

void* get_context_val(WsContextH hCntx, char* name, int* size, unsigned long type);
void* ws_get_context_val(WsContextH cntx, char* name, int* size);
unsigned long ws_get_context_ulong_val(WsContextH cntx, char* name);

WsContextH ws_create_runtime (GList *interfaces);

int wsman_register_interface(WsContextH cntx, 
        WsDispatchInterfaceInfo* wsInterface,
        WsManDispatcherInfo* dispInfo);

int wsman_register_endpoint(WsContextH cntx, 
        WsDispatchInterfaceInfo* wsInterface,
        WsDispatchEndPointInfo* ep,
        WsManDispatcherInfo* dispInfo);
        


int ws_transfer_put(SoapOpH op, void* appData);        
int wsmid_identify_stub(SoapOpH op, void* appData);        
int ws_enumerate_stub(SoapOpH op, void* appData);        
int ws_transfer_get(SoapOpH op, void* appData);
int ws_transfer_get_raw(SoapOpH op, void* appData);
int ws_pull_stub(SoapOpH op, void* appData);
int ws_pull_stub_raw(SoapOpH op, void* appData);
int ws_release_stub(SoapOpH op, void* appData);
WsEnumerateInfo* get_enum_info(WsContextH cntx, 
        WsXmlDocH doc, 
        char* cntxName,
        int cntxNameLen,
        char* op,
        char** enumIdPtr);



SoapOpH soap_create_op(SoapH soap, char *inboundAction, char *outboundAction, char *role, SoapServiceCallback callbackProc, void *callbackData, unsigned long flags, unsigned long timeout);
int soap_add_op_filter(SoapOpH op, SoapServiceCallback proc, void *data, int inbound);
WsXmlDocH soap_get_op_doc(SoapOpH op, int inbound);
WsXmlDocH soap_detach_op_doc(SoapOpH op, int inbound);
int soap_set_op_doc(SoapOpH op, WsXmlDocH doc, int inbound);
char *soap_get_op_action(SoapOpH op, int inbound);
void soap_set_op_action(SoapOpH op, char *action, int inbound);
unsigned long soap_get_op_flags(SoapOpH op);
SoapH soap_get_op_soap(SoapOpH op);
char *soap_get_op_dest_url(SoapOpH op);
void soap_mark_processed_op_header(SoapOpH h, WsXmlNodeH xmlNode);
void soap_destroy_op(SoapOpH op);


int outbound_addressing_filter(SoapOpH opHandle, void* data);
int outbound_control_header_filter(SoapOpH opHandle, void* data);

WsContextH ws_create_ep_context(SoapH soap, WsXmlDocH doc);

WsContextH ws_get_soap_context(SoapH soap);

void ws_clear_context_entries(WsContextH hCntx);

int ws_destroy_context(WsContextH hCntx);

int ws_remove_context_val(WsContextH hCntx, char* name);

int ws_set_context_val(WsContextH hCntx, char* name, void* val, int size, int bNoDup);
int set_context_val(WsContextH hCntx, 
        char* name, 
        void* val, 
        int size, 
        int bNoDup, 
        unsigned long type);
int ws_set_context_xml_doc_val(WsContextH cntx, char* name, WsXmlDocH val);

void destroy_context_entry(WS_CONTEXT_ENTRY* entry);
WS_CONTEXT_ENTRY* create_context_entry(DL_List* list, 
        char* name, 
        void* val,
        unsigned long size,
        unsigned long flags);
        
int do_serializer_free(WsContextH cntx, void* ptr);
int ws_serializer_free(WsContextH cntx, void* ptr);

void ws_serializer_free_all(WsContextH cntx);
int soap_add_disp_filter(SoapDispatchH disp,
        SoapServiceCallback callbackProc,
        void* callbackData,
        int inbound);

int ws_set_context_ulong_val(WsContextH cntx, char* name, unsigned long val);

int soap_add_defalt_filter(SoapH soap,
        SoapServiceCallback callbackProc,
        void* callbackData,
        int inbound);

char* get_http_response_status(WsXmlDocH doc, int* errCodePtr);

int wsman_fault_occured(WsmanMessage *msg);

WsXmlDocH wsman_generate_fault(
	WsContextH cntx,
	WsXmlDocH inDoc, 
	WsmanFaultCodeType faultCode, 
	WsmanFaultDetailType faultDetail,
        char *fault_msg);
	
void wsman_generate_fault_buffer(
	WsContextH cntx, 
	WsXmlDocH inDoc, 
	WsmanFaultCodeType faultCode, 
	WsmanFaultDetailType faultDetail, 
        char *fault_msg,
	char **buf, 
	int* len);


int soap_xml_wait_for_response(SoapOpH op, unsigned long tm);				

        
 
// ******************** 

#define WS_DISP_TYPE_MASK				0xffff

#define WS_DISP_TYPE_RAW_DOC				0
#define WS_DISP_TYPE_GET				1
#define WS_DISP_TYPE_PUT				2
#define WS_DISP_TYPE_CREATE				3
#define WS_DISP_TYPE_DELETE				4

#define WS_DISP_TYPE_ENUMERATE		        	5
#define WS_DISP_TYPE_PULL				6
#define WS_DISP_TYPE_RELEASE				7
#define WS_DISP_TYPE_UPDATE				8
#define WS_DISP_TYPE_GETSTATUS			        9
#define WS_DISP_TYPE_COUNT				11
#define WS_DISP_TYPE_PULL_RAW			        12
#define WS_DISP_TYPE_GET_RAW				13
#define WS_DISP_TYPE_GET_NAMESPACE			14
#define WS_DISP_TYPE_CUSTOM_METHOD			15
#define WS_DISP_TYPE_PUT_RAW				16
#define WS_DISP_TYPE_IDENTIFY				17

#define WS_DISP_TYPE_PRIVATE				0xfffe



#define END_POINT_IDENTIFY(t, ns)\
	{ WS_DISP_TYPE_IDENTIFY, NULL, NULL, NULL, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Identify_EP, ns, NULL}

#define END_POINT_TRANSFER_GET(t, ns)\
	{ WS_DISP_TYPE_GET, NULL, NULL, TRANSFER_ACTION_GET, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Get_EP, ns, t##_Get_Selectors}

#define END_POINT_TRANSFER_GET_RAW(t, ns)\
	{ WS_DISP_TYPE_GET_RAW, NULL, NULL, TRANSFER_ACTION_GET, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Get_EP, ns, NULL}	  

#define END_POINT_TRANSFER_PUT_RAW(t, ns)\
	{ WS_DISP_TYPE_PUT_RAW, NULL, NULL, TRANSFER_ACTION_PUT, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Put_EP, ns, NULL}	  

#define END_POINT_TRANSFER_GET_NAMESPACE(t, ns)\
	{ WS_DISP_TYPE_GET_NAMESPACE, NULL, NULL, TRANSFER_ACTION_GET, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Get_EP, ns, NULL}	  

#define END_POINT_TRANSFER_PUT(t, ns)\
	{ WS_DISP_TYPE_PUT, NULL, NULL, TRANSFER_ACTION_PUT, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Put_EP, ns, NULL}




#define END_POINT_TRANSFER_ENUMERATE(t, ns)\
	{ WS_DISP_TYPE_ENUMERATE, NULL, NULL, ENUM_ACTION_ENUMERATE, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Enumerate_EP, ns, NULL}

#define END_POINT_TRANSFER_RELEASE(t, ns)\
	{ WS_DISP_TYPE_RELEASE, NULL, NULL, ENUM_ACTION_RELEASE, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Release_EP, ns, NULL}

#define END_POINT_TRANSFER_PULL(t, ns)\
	{ WS_DISP_TYPE_PULL, NULL, NULL, ENUM_ACTION_PULL, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Pull_EP, ns, NULL}

#define END_POINT_TRANSFER_PULL_RAW(t, ns)\
	{ WS_DISP_TYPE_PULL_RAW, NULL, NULL, ENUM_ACTION_PULL, NULL,\
	  t##_TypeInfo, (WsProcType)t##_Pull_EP, ns, NULL}	  

#define END_POINT_PRIVATE_EP(t, a, m, ns)    \
        { WS_DISP_TYPE_PRIVATE, NULL, NULL, a, NULL, \
          t##_TypeInfo, (WsProcType)t##_##m##_EP, ns, NULL }

#define END_POINT_CUSTOM_METHOD(t, ns)    \
        { WS_DISP_TYPE_PRIVATE, NULL, NULL, NULL, NULL, \
          t##_TypeInfo, (WsProcType)t##_Custom_EP, ns, NULL }

#define END_POINT_LAST	{ 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
#define NAMESPACE_LAST	{ NULL }

#define ADD_NAMESPACE( ns)\
        {ns}


#define WS_STUB_0(T)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, &(d->retVal));\
} 

#define WS_STUB_1(T, n1)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, &(d->retVal));\
} 

#define WS_STUB_2(T, n1, n2)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, d->n2, &(d->retVal));\
} 

#define WS_STUB_3(T, n1, n2, n3)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, d->n2, d->n3, &(d->retVal));\
} 

#define WS_STUB_4(T, n1, n2, n3, n4)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, d->n2, d->n3, d->n4, &(d->retVal));\
} 

#define WS_STUB_5(T, n1, n2, n3, n4, n5)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, d->n2, d->n3, d->n4, d->n5, &(d->retVal));\
} 


#define WS_STUB_NO_RET_0(T)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c);\
} 

#define WS_STUB_NO_RET_1(T, n1)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1);\
} 

#define WS_STUB_NO_RET_2(T, n1, n2)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, d->n2);\
} 

#define WS_STUB_NO_RET_3(T, n1, n2, n3)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, d->n2, d->n3);\
} 

#define WS_STUB_NO_RET_4(T, n1, n2, n3, n4)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, d->n2, d->n3, d->n4);\
} 

#define WS_STUB_NO_RET_5(T, n1, n2, n3, n4, n5)\
int T##_stub(WsContextH c, void* ptr,  XmlSerializerInfo* s)\
{\
	struct T##_ProcInfo* d = (struct T##_ProcInfo*)ptr;\
	return T##_EP(c, d->n1, d->n2, d->n3, d->n4, d->n5);\
} 




// Selectors
#define ADD_SELECTOR(n,t,d)\
	{ n, NULL, t, d}
	  
#define SELECTOR_LAST { NULL, NULL, NULL, NULL }
#define START_TRANSFER_GET_SELECTORS(t) WsSelector t##_Get_Selectors[] = {

#define FINISH_TRANSFER_GET_SELECTORS(t) SELECTOR_LAST }
#define DECLARE_SELECTOR_ARRAY(t)\
extern WsSelector t##_Get_Selectors[]




void soap_enter(SoapH soap);
void soap_leave(SoapH soap);
char *wsman_remove_query_string(char * resourceUri);
void soap_destroy_fw(SoapH soap);

int wsen_get_max_elements(WsContextH cntx, WsXmlDocH doc);
void wsman_set_estimated_total(WsXmlDocH in_doc, WsXmlDocH out_doc, WsEnumerateInfo *enumInfo);
int wsman_is_optimization(WsContextH cntx, WsXmlDocH doc);
char * wsman_get_enum_mode(WsContextH cntx, WsXmlDocH doc);
void wsman_set_enum_mode(char *enum_mode, WsEnumerateInfo *enumInfo);

int wsman_is_fault_envelope(WsXmlDocH doc);

void wsman_set_fault(WsmanMessage *msg, 
            WsmanFaultCodeType fault_code, 
            WsmanFaultDetailType fault_detail_code,
            const char *details);

int wsman_is_identify_request(WsXmlDocH doc);


#endif /*SOAP_API_H_*/
