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

#include "wsman-xml-api.h"


struct __WsXmlParserData
{
	WsXmlDocH nsHolder;
	void* _private;	
};
typedef struct __WsXmlParserData WsXmlParserData;



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


extern WsXmlNsData g_wsNsData[];
extern WsManDialectData g_wsDialectData[];



WsXmlDocH ws_xml_create_envelope(SoapH soap, char *soapVersion);

WsXmlDocH ws_xml_duplicate_doc(SoapH dstSoap, WsXmlDocH srcDoc);


/* 
char* ws_xml_find_text_in_doc(WsXmlDocH doc, char* nsUri, char* name); 
char* ws_xml_find_text_in_tree(WsXmlNodeH head, char* nsUri, char* name, int bRecursive);
*/

WsXmlDocH ws_xml_create_doc_by_import( WsXmlNodeH node );


SoapH ws_xml_get_doc_soap_handle(WsXmlDocH doc);
/*
WsXmlNodeH ws_xml_get_soap_fault(WsXmlDocH doc);
WsXmlNodeH ws_xml_get_soap_operation(WsXmlDocH doc);
*/
int ws_xml_parser_initialize(SoapH soap, WsXmlNsData nsData[]);
void ws_xml_parser_destroy(SoapH soap);

int ws_xml_get_child_count(WsXmlNodeH parent);

char* ws_xml_get_node_name_ns_uri(WsXmlNodeH node);

char* ws_xml_get_node_name_ns_prefix(WsXmlNodeH node);

WsXmlDocH ws_xml_read_file(SoapH soap, char* filename, char* encoding, unsigned long options);
WsXmlDocH ws_xml_read_memory(SoapH soap, char *buf, int size, char *encoding, unsigned long options);

WsXmlDocH ws_xml_create_doc(SoapH soap, char *rootNsUri, char *rootName);
WsXmlNsH ws_xml_find_wk_ns(SoapH soap, char *uri, char *prefix);

WsXmlNsH ws_xml_ns_add(WsXmlNodeH node, char* uri, char* prefix);

WsXmlNodeH ws_xml_add_child_format(WsXmlNodeH node, char* nsUri, char* localName, char* format, ...);

WsXmlNodeH ws_xml_add_empty_child_format(WsXmlNodeH node, char* nsUri, char* format, ...);

int check_xpath(WsXmlNodeH node, char *xpath_expr);

int ws_xml_utf8_strlen(char *buf);


void ws_xml_set_node_lang(WsXmlNodeH node, char* lang);

void ws_xml_copy_node(WsXmlNodeH src, WsXmlNodeH dst);

/** @} */

#endif /*XML_API_GENERIC_H_*/
