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


/**
 * @addtogroup XMLParserGeneric
 * 
 * @{
 */

#ifndef XML_API_GENERIC_H_
#define XML_API_GENERIC_H_



#define XML_LAST_CHILD			(-1)
#define XML_ELEMENT_NEXT		(-2)
#define XML_ELEMENT_PREV		(-3)
#define XML_ELEMENT_PARENT		(-4)


#define XML_NS_URI				1
#define XML_NS_PREFIX			2
#define XML_LOCAL_NAME			3
#define XML_TEXT_VALUE			4
  
#define XML_COUNT_NODE			10
#define XML_COUNT_NS			11
#define XML_COUNT_ATTR			12

struct __WsXmlParserData
{
	WsXmlDocH nsHolder;
	void* _private;	
};
typedef struct __WsXmlParserData WsXmlParserData;


struct __internalWsDoc
{
	void* parserDoc;
	SOAP_FW* fw;
	unsigned long prefixIndex;
};
typedef struct __internalWsDoc iWsDoc;


#define xml_parser_get_first_child(x) xml_parser_node_get(x, 0)
#define xml_parser_get_next_child(x) xml_parser_node_get(x, XML_ELEMENT_NEXT)



struct _WsXmlFindNsData
{
    char* prefix;
    char* nsUri;
    WsXmlNodeH node;
    WsXmlNsH ns;
};
typedef struct _WsXmlFindNsData WsXmlFindNsData;

struct _FindInTreeCallbackData
{
    char* ns;
    char* name;
    WsXmlNodeH node;
};
typedef struct _FindInTreeCallbackData FindInTreeCallbackData;

struct __WsXmlDumpNodeTreeData
{
    FILE* stream;
    int indent;
};
typedef struct __WsXmlDumpNodeTreeData WsXmlDumpNodeTreeData;




WsXmlNodeH ws_xml_get_soap_operation(WsXmlDocH doc);


WsXmlDocH ws_xml_create_envelope(SoapH soap, char *soapVersion);
WsXmlDocH ws_xml_duplicate_doc(SoapH dstSoap, WsXmlDocH srcDoc);
void ws_xml_duplicate_attr(WsXmlNodeH dstNode, WsXmlNodeH srcNode);
void ws_xml_duplicate_children(WsXmlNodeH dstNode, WsXmlNodeH srcNode);
void ws_xml_duplicate_tree(WsXmlNodeH dstNode, WsXmlNodeH srcNode);
void epr_from_request_to_response(WsXmlNodeH dstHeader, WsXmlNodeH epr);
WsXmlDocH ws_create_response_envelope(struct __WsContext *cntx, WsXmlDocH rqstDoc, char *action);
WsXmlDocH ws_xml_create_fault(WsContextH cntx, WsXmlDocH rqstDoc, char *code, char *subCodeNs, char *subCode, char *lang, char *reason, void (*addDetailProc)(WsXmlNodeH, void *), void *addDetailProcData);
int ws_xml_parser_initialize(SoapH soap, WsXmlNsData nsData[]);
WsXmlNodeH ws_xml_get_soap_header(WsXmlDocH doc);
int ws_xml_enum_children(WsXmlNodeH parent, WsXmlEnumCallback callback, void *data, int bRecursive);
int ws_xml_get_child_count(WsXmlNodeH parent);
int ws_xml_enum_tree(WsXmlNodeH top, WsXmlEnumCallback callback, void *data, int bRecursive);
char *ws_xml_get_node_name_ns(WsXmlNodeH node);
char *ws_xml_get_node_local_name(WsXmlNodeH node);
WsXmlNodeH ws_xml_get_doc_root(WsXmlDocH doc);
char *ws_xml_get_node_text(WsXmlNodeH node);
WsXmlDocH ws_xml_read_memory(SoapH soap, char *buf, int size, char *encoding, unsigned long options);
WsXmlDocH ws_xml_create_doc(SoapH soap, char *rootNsUri, char *rootName);
int ws_xml_set_node_name(WsXmlNodeH node, char *nsUri, char *name);
void ws_xml_destroy_doc(WsXmlDocH doc);
int find_in_tree_callback(WsXmlNodeH node, void *_data);
WsXmlNodeH ws_xml_find_in_tree(WsXmlNodeH head, char *nsUri, char *localName, int bRecursive);
WsXmlNodeH ws_xml_get_soap_body(WsXmlDocH doc);
WsXmlNodeH ws_xml_get_soap_element(WsXmlDocH doc, char *name);
WsXmlNodeH ws_xml_get_child(WsXmlNodeH parent, int index, char *nsUri, char *localName);
int ws_xml_is_node_qname(WsXmlNodeH node, char *nsUri, char *name);
void make_default_prefix(WsXmlNodeH node, char *uri, char *buf, int bufsize);
WsXmlNsH ws_xml_find_wk_ns(SoapH soap, char *uri, char *prefix);
WsXmlNodeH ws_xml_get_soap_envelope(WsXmlDocH doc);
WsXmlNodeH ws_xml_get_node_parent(WsXmlNodeH node);
void ws_xml_ns_enum(WsXmlNodeH node, WsXmlNsEnumCallback callback, void *data, int bWalkUpTree);
WsXmlNsH ws_xml_find_ns(WsXmlNodeH node, char *nsUri, char *prefix, int bWalkUpTree);
int ws_xml_find_ns_callback(WsXmlNodeH node, WsXmlNsH ns, void *_data);
int ws_xml_get_ns_count(WsXmlNodeH node, int bWalkUpTree);
char *ws_xml_get_ns_prefix(WsXmlNsH ns);
char *ws_xml_get_ns_uri(WsXmlNsH ns);
WsXmlNsH ws_xml_get_ns(WsXmlNodeH node, int index);
int ns_enum_at_node(WsXmlNodeH node, WsXmlNsEnumCallback callback, void *data);
WsXmlNodeH ws_xml_add_child(WsXmlNodeH node, char *ns, char *localName, char *val);
int is_ns_prefix_ok(WsXmlNsH ns, char *newPrefix, int bDefault);
WsXmlNsH ws_xml_define_ns(WsXmlNodeH node, char *nsUri, char *nsPrefix, int bDefault);
WsXmlNodeH ws_xml_add_qname_child(WsXmlNodeH parent, char *nameNs, char *name, char *valueNs, char *value);
WsXmlAttrH ws_xml_add_qname_attr(WsXmlNodeH node, char *nameNs, char *name, char *valueNs, char *value);
int ws_xml_get_node_attr_count(WsXmlNodeH node);
WsXmlAttrH ws_xml_add_node_attr(WsXmlNodeH node, char *nsUri, char *name, char *value);
void ws_xml_remove_node_attr(WsXmlAttrH attr);
WsXmlAttrH ws_xml_get_node_attr(WsXmlNodeH node, int index);
WsXmlAttrH ws_xml_find_node_attr(WsXmlNodeH node, char *attrNs, char *attrName);
unsigned long ws_xml_get_node_ulong(WsXmlNodeH node);
int ws_xml_set_node_ulong(WsXmlNodeH node, unsigned long uVal);
char *make_qname(WsXmlNodeH node, char *uri, char *name);
char *ws_xml_get_attr_name(WsXmlAttrH attr);
char *ws_xml_get_attr_ns(WsXmlAttrH attr);
char *ws_xml_get_attr_ns_prefix(WsXmlAttrH attr);
char *ws_xml_get_attr_value(WsXmlAttrH attr);
char *ws_xml_find_attr_value(WsXmlNodeH node, char *ns, char *attrName);
int ws_xml_find_attr_bool(WsXmlNodeH node, char *ns, char *attrName);
unsigned long ws_xml_find_attr_ulong(WsXmlNodeH node, char *ns, char *attrName);
int ws_xml_set_node_qname_val(WsXmlNodeH node, char *valNsUri, char *valName);
WsXmlDocH ws_xml_get_node_doc(WsXmlNodeH node);
int ws_xml_set_node_text(WsXmlNodeH node, char *text);
int is_root_node(WsXmlNodeH node);
int is_xml_val_true(char *text);
void ws_dump_xml_strings(FILE *f, char *str1, char *str2, char *str3, char *str4);
void ws_dump_indent(FILE *f, int indent);
int ws_dump_node_attrs(FILE *f, WsXmlNodeH node, int indent);
int ws_dump_node_ns_list(FILE *f, WsXmlNodeH node, int indent, int attrCount);
void ws_do_dump_xml_node(WsXmlNodeH node, WsXmlDumpNodeTreeData *data);
int ws_xml_dump_node_tree_callback(WsXmlNodeH node, void *_data);
void ws_xml_dump_node_tree(FILE *f, WsXmlNodeH node, int bRecursive);

char* ws_xml_find_text_in_doc(WsXmlDocH doc, char* nsUri, char* name);
char* ws_xml_find_text_in_tree(WsXmlNodeH head, char* nsUri, char* name, int bRecursive);
void ws_xml_free_memory(void* ptr);

void ws_xml_dump_memory_enc(WsXmlDocH doc, char** buf, int* ptrSize, char* encoding);

SoapH ws_xml_get_doc_soap_handle(WsXmlDocH doc);

WsXmlNodeH ws_xml_get_soap_fault(WsXmlDocH doc);


int ws_xml_get_child_count(WsXmlNodeH parent);


/** @} */

#endif /*XML_API_GENERIC_H_*/
