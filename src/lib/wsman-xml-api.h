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


#ifndef _SoapH_Defined
#define _SoapH_Defined
struct __Soap
{
	unsigned __undefined;
};
typedef struct __Soap* SoapH;
#endif


struct __WsXmlDoc
{
	int __undefined;
};
typedef struct __WsXmlDoc* WsXmlDocH;

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

WsXmlNodeH ws_xml_get_doc_root(WsXmlDocH doc);

void ws_xml_destroy_doc(WsXmlDocH doc);

#endif /*WS_XML_API_H_*/
