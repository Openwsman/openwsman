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
#ifndef WS_XML_API_H_
#define WS_XML_API_H_

#include "u/libu.h"


#define SOAP_ENVELOPE			"Envelope"
#define SOAP_HEADER				"Header"
#define SOAP_BODY				"Body"
#define SOAP_FAULT				"Fault"
#define SOAP_CODE				"Code"
#define SOAP_VALUE				"Value"
#define SOAP_SUBCODE			"Subcode"
#define SOAP_REASON				"Reason"
#define SOAP_TEXT				"Text"
#define SOAP_LANG				"lang"
#define SOAP_DETAIL				"Detail"
#define SOAP_MUST_UNDERSTAND	"mustUnderstand"
#define SOAP_VERSION_MISMATCH	"VersionMismatch"
#define SOAP_UPGRADE			"Upgrade"
#define SOAP_SUPPORTED_ENVELOPE	"SupportedEnvelope"


#define XML_LAST_CHILD          (-1)
#define XML_ELEMENT_NEXT        (-2)
#define XML_ELEMENT_PREV        (-3)
#define XML_ELEMENT_PARENT      (-4)


#define XML_NS_URI              1
#define XML_NS_PREFIX           2
#define XML_LOCAL_NAME          3
#define XML_TEXT_VALUE          4
  
#define XML_COUNT_NODE          10
#define XML_COUNT_NS            11
#define XML_COUNT_ATTR          12


struct _WsXmlDoc;
typedef struct _WsXmlDoc* WsXmlDocH;

 struct WS_CONTEXT;
 typedef struct WS_CONTEXT* WsContextH;
 

struct __Soap;
typedef struct __Soap* SoapH;






struct __WsXmlNode
{
	int __undefined;
};
typedef struct __WsXmlNode* WsXmlNodeH;

struct __WsXmlAttr
{
	int __undefined;
};
typedef struct __WsXmlAttr* WsXmlAttrH;

struct __WsXmlNs
{
	int __undefined;
};
typedef struct __WsXmlNs* WsXmlNsH;

struct __WsXmlNsData
{
	char* uri;
	char* prefix;
};
typedef struct __WsXmlNsData WsXmlNsData;

extern WsXmlNsData g_wsNsData[];

struct __WsManDialectData
{
	char* uri;
	char* name;
};
typedef struct __WsManDialectData WsManDialectData;

extern WsXmlNsData g_wsNsData[];
extern WsManDialectData g_wsDialectData[];


typedef int (*WsXmlEnumCallback)(WsXmlNodeH, void*);
typedef int (*WsXmlNsEnumCallback)(WsXmlNodeH, WsXmlNsH, void*);

void ws_xml_dump_node_tree(FILE* f, WsXmlNodeH node);
void ws_xml_dump_memory_node_tree(WsXmlNodeH node, char** buf, int* ptrSize);
void ws_xml_dump_memory_enc(WsXmlDocH doc, char** buf, int* ptrSize, char* encoding);

WsXmlNodeH ws_xml_get_doc_root(WsXmlDocH doc);

void ws_xml_destroy_doc(WsXmlDocH doc);

char *ws_xml_get_xpath_value (WsXmlDocH doc, char *expression);


void ws_xml_duplicate_attr(WsXmlNodeH dstNode, WsXmlNodeH srcNode);

void ws_xml_duplicate_children(WsXmlNodeH dstNode, WsXmlNodeH srcNode);

void ws_xml_duplicate_tree(WsXmlNodeH dstNode, WsXmlNodeH srcNode);


WsXmlNodeH ws_xml_get_soap_header(WsXmlDocH doc);

int ws_xml_enum_children(WsXmlNodeH parent, WsXmlEnumCallback callback, void *data, int bRecursive);

int ws_xml_get_child_count(WsXmlNodeH parent);

int ws_xml_enum_tree(WsXmlNodeH top, WsXmlEnumCallback callback, void *data, int bRecursive);

char *ws_xml_get_node_name_ns(WsXmlNodeH node);

char *ws_xml_get_node_local_name(WsXmlNodeH node);

WsXmlNodeH ws_xml_get_doc_root(WsXmlDocH doc);

char *ws_xml_get_node_text(WsXmlNodeH node);


int ws_xml_set_node_name(WsXmlNodeH node, char *nsUri, char *name);


WsXmlNodeH ws_xml_find_in_tree(WsXmlNodeH head, char *nsUri, char *localName, int bRecursive);

WsXmlNodeH ws_xml_get_soap_body(WsXmlDocH doc);

WsXmlNodeH ws_xml_get_soap_element(WsXmlDocH doc, char *name);

WsXmlNodeH ws_xml_get_child(WsXmlNodeH parent, int index, char *nsUri, char *localName);

int ws_xml_is_node_qname(WsXmlNodeH node, char *nsUri, char *name);


WsXmlNodeH ws_xml_get_soap_envelope(WsXmlDocH doc);

WsXmlNodeH ws_xml_get_node_parent(WsXmlNodeH node);

void ws_xml_ns_enum(WsXmlNodeH node, WsXmlNsEnumCallback callback, void *data, int bWalkUpTree);

WsXmlNsH ws_xml_find_ns(WsXmlNodeH node, char *nsUri, char *prefix, int bWalkUpTree);

// int ws_xml_find_ns_callback(WsXmlNodeH node, WsXmlNsH ns, void *_data);

int ws_xml_get_ns_count(WsXmlNodeH node, int bWalkUpTree);

char *ws_xml_get_ns_prefix(WsXmlNsH ns);

char *ws_xml_get_ns_uri(WsXmlNsH ns);

WsXmlNsH ws_xml_get_ns(WsXmlNodeH node, int index);

WsXmlNodeH ws_xml_add_child(WsXmlNodeH node, char *ns, char *localName, char *val);

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

void ws_xml_make_default_prefix(WsXmlNodeH node, char* uri, char* buf, int bufsize);

void ws_xml_dump_doc(FILE* f, WsXmlDocH doc );

#endif /*WS_XML_API_H_*/
