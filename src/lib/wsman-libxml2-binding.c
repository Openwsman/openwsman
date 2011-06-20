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
 */
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <assert.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>


#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-binding.h"


static void destroy_attr_private_data(void *data)
{
	if (data)
		xmlFree(data);
}


static void destroy_node_private_data(void *_data)
{
	iWsNode *data = (iWsNode *) _data;
	if (data) {
		// ??? TBD data->nsQNameList;
		if (data->valText)
			xmlFree(data->valText);
		u_free(data);
	}
}

static void destroy_tree_private_data(xmlNode * node)
{
	while (node) {
		xmlAttrPtr attr = node->properties;

		if (node->_private) {
			destroy_node_private_data(node->_private);
			node->_private = NULL;
		}

		while (attr) {
			if (attr->_private) {
				destroy_attr_private_data(attr->_private);
				attr->_private = NULL;
			}
			attr = attr->next;
		}
		destroy_tree_private_data(node->children);
		node = node->next;
	}
}

static void
myXmlErrorReporting (void *ctx, const char* msg, ...)
{
	va_list args;
	char *string;
	va_start(args, msg);
	string = u_strdup_vprintf (msg, args);
	warning (string);
	va_end(args);

	u_free(string);
}

void xml_parser_initialize()
{
	xmlSetGenericErrorFunc(NULL, myXmlErrorReporting);
}

void xml_parser_destroy()
{
}

int xml_parser_utf8_strlen(char *buf)
{
	return xmlUTF8Strlen(BAD_CAST buf);
}

void xml_parser_doc_to_memory(WsXmlDocH doc, char **buf,
		int *ptrSize, const char *encoding)
{
	if (doc && buf && ptrSize)
		xmlDocDumpMemoryEnc(doc->parserDoc,
				(xmlChar **) buf, ptrSize,
				(!encoding) ? "UTF-8" : encoding);
}

void xml_parser_free_memory(void *ptr)
{
	if (ptr)
		xmlFree(ptr);
}



int xml_parser_create_doc_by_import(WsXmlDocH wsDoc, WsXmlNodeH node)
{
	xmlDocPtr doc;
	xmlNodePtr rootNode;

	if ((doc = xmlNewDoc(BAD_CAST "1.0")) == NULL) {
		if (doc)
			xmlFreeDoc(doc);
		return 0;
	} else {
		doc->_private = wsDoc;
		wsDoc->parserDoc = doc;
		rootNode = xmlDocCopyNode((xmlNodePtr) node, doc, 1);
		xmlDocSetRootElement(doc, rootNode);
		return 1;
	}
}

int xml_parser_create_doc(WsXmlDocH wsDoc, const char *rootName)
{
	int retVal = 0;
	xmlDocPtr doc;
	xmlNodePtr rootNode;

	if ((doc = xmlNewDoc(BAD_CAST "1.0")) == NULL ||
			(rootNode = xmlNewNode(NULL, BAD_CAST rootName)) == NULL) {
		if (doc)
			xmlFreeDoc(doc);
	} else {
		doc->_private = wsDoc;
		wsDoc->parserDoc = doc;
		xmlDocSetRootElement(doc, rootNode);
		retVal = 1;
	}

	return retVal;
}



void xml_parser_destroy_doc(WsXmlDocH wsDoc)
{
	xmlDocPtr xmlDoc = (xmlDocPtr) wsDoc->parserDoc;
	if (xmlDoc != NULL) {
		destroy_tree_private_data(xmlDocGetRootElement(xmlDoc));
		xmlFreeDoc(xmlDoc);
	}
}


WsXmlDocH xml_parser_get_doc(WsXmlNodeH node)
{
	xmlDocPtr xmlDoc = ((xmlDocPtr) node)->doc;
	return (WsXmlDocH) (!xmlDoc ? NULL : xmlDoc->_private);
}


WsXmlNodeH xml_parser_get_root(WsXmlDocH doc)
{
	if (doc->parserDoc != NULL)
		return (WsXmlNodeH) xmlDocGetRootElement((xmlDocPtr) doc->
				parserDoc);
	return NULL;
}

WsXmlDocH
xml_parser_file_to_doc( const char *filename,
		const char *encoding, unsigned long options)
{
	xmlDocPtr xmlDoc;
	WsXmlDocH Doc = NULL;

	xmlDoc = xmlReadFile(filename, encoding,
			XML_PARSE_NONET | XML_PARSE_NSCLEAN);
	if (xmlDoc == NULL) {
		return NULL;
	}
	Doc = (WsXmlDocH) u_zalloc(sizeof(*Doc));
	if (Doc == NULL) {
		xmlFreeDoc(xmlDoc);
		return NULL;
	}
	xmlDoc->_private = Doc;
	Doc->parserDoc = xmlDoc;

	return Doc;

}

WsXmlDocH
xml_parser_memory_to_doc( const char *buf, size_t size,
		const char *encoding, unsigned long options)
{
	WsXmlDocH Doc = NULL;
	xmlDocPtr xmlDoc;
	if (!buf || !size ) {
		return NULL;
	}
	xmlDoc = xmlReadMemory(buf, (int) size, NULL, encoding,
			XML_PARSE_NONET | XML_PARSE_NSCLEAN);
	if (xmlDoc == NULL) {
		return NULL;
	}
	Doc = (WsXmlDocH) u_zalloc(sizeof(*Doc));
	if (Doc == NULL) {
		xmlFreeDoc(xmlDoc);
		return NULL;
	}

	xmlDoc->_private = Doc;
	Doc->parserDoc = xmlDoc;

	return Doc;
}


char *xml_parser_node_query(WsXmlNodeH node, int what)
{
	char *ptr = NULL;
	xmlNodePtr xmlNode = (xmlNodePtr) node;
	iWsNode *wsNode = (iWsNode *) xmlNode->_private;

	switch (what) {
	case XML_TEXT_VALUE:
		if (wsNode == NULL)
			xmlNode->_private = wsNode =
				u_zalloc(sizeof(iWsNode));

		if (wsNode != NULL) {
			if (wsNode->valText == NULL) {
				wsNode->valText =
					(char *) xmlNodeGetContent(xmlNode);
			}
			ptr = wsNode->valText;
		}
		break;
	case XML_LOCAL_NAME:
		ptr = (char *) xmlNode->name;
		break;
	case XML_NS_URI:
		if (xmlNode->ns != NULL)
			ptr = (char *) xmlNode->ns->href;
		break;
	case XML_NS_PREFIX:
		if (xmlNode->ns != NULL)
			ptr = (char *) xmlNode->ns->prefix;
		break;
	default:
		break;
	}

	return ptr;
}


int xml_parser_node_set(WsXmlNodeH node, int what, const char *str)
{
	int retVal = -1;
	xmlNodePtr xmlNode = (xmlNodePtr) node;
	iWsNode *wsNode = (iWsNode *) xmlNode->_private;
	xmlNsPtr xmlNs;

	switch (what) {
	case XML_TEXT_VALUE:
		if (wsNode == NULL)
			xmlNode->_private = wsNode =
				u_zalloc(sizeof(iWsNode));

		if (wsNode != NULL) {
			if (wsNode->valText != NULL) {
				xmlFree(wsNode->valText);
				wsNode->valText = NULL;
			}
			xmlNodeSetContent(xmlNode, BAD_CAST str);
			retVal = 0;
		}
		break;

	case XML_LOCAL_NAME:
		xmlNodeSetName(xmlNode, BAD_CAST str);
		retVal = 0;
		break;

	case XML_NS_URI:
		if ((xmlNs =
					(xmlNsPtr) xml_parser_ns_find(node, str, NULL, 1,
						1)) != NULL) {
			xmlNode->ns = xmlNs;
			retVal = 0;
		} else
			retVal = 1;
		break;
	default:
		retVal = 1;
		break;
	}

	return retVal;
}



WsXmlNodeH xml_parser_node_get(WsXmlNodeH node, int which)
{
	xmlNodePtr xmlNode = NULL;
	xmlNodePtr base = (xmlNodePtr) node;

	switch (which) {
	case XML_ELEMENT_PARENT:
		xmlNode = base->parent;
		break;
	case XML_ELEMENT_NEXT:
		if ((xmlNode = base->next) != NULL) {
			do {
				if (xmlNode->type == XML_ELEMENT_NODE)
					break;
			}
			while ((xmlNode = xmlNode->next) != NULL);
		}
		break;
	case XML_ELEMENT_PREV:
		if ((xmlNode = base->prev) != NULL) {
			do {
				if (xmlNode->type == XML_ELEMENT_NODE)
					break;
			}
			while ((xmlNode = xmlNode->prev) != NULL);
		}
		break;
	case XML_LAST_CHILD:
	default:
		if (which >= 0 || which == XML_LAST_CHILD) {
			int count = 0;
			xmlNode = base->children;

			while (xmlNode) {
				if (xmlNode->type == XML_ELEMENT_NODE) {
					if (which == XML_LAST_CHILD &&
							xmlNode->next == NULL)
						break;

					if (count == which)
						break;

					count++;
				}
				xmlNode = xmlNode->next;
			}
		} else {
			assert(which >= 0);
		}
		break;
	}
	return (WsXmlNodeH) xmlNode;
}


WsXmlNsH
xml_parser_ns_find(WsXmlNodeH node,
		const char *uri,
		const char *prefix,
		int bWalkUpTree, int bAddAtRootIfNotFound)
{
	xmlNodePtr xmlNode = (xmlNodePtr) node;
	xmlNsPtr xmlNs = NULL;

	while (xmlNode != NULL) {
		xmlNs = xmlNode->nsDef;
		while (xmlNs != NULL) {
			if (uri) {
				if (!strcmp((char *) xmlNs->href, uri))
					break;
			} else if (prefix == NULL) {
				if (xmlNs->prefix == NULL)
					break;
			} else
				if (xmlNs->prefix &&
						!strcmp((char *) xmlNs->prefix, prefix)) {
					break;
				}
			xmlNs = xmlNs->next;
		}
		if (xmlNs != NULL || !bWalkUpTree)
			break;
		xmlNode = xmlNode->parent;
	}

	if (xmlNs == NULL && bAddAtRootIfNotFound) {
		xmlNodePtr xmlRoot =
			xmlDocGetRootElement(((xmlDocPtr) node)->doc);
		char buf[12];

		if (prefix == NULL) {
			ws_xml_make_default_prefix((WsXmlNodeH) xmlRoot,
					uri, buf, sizeof(buf));
			prefix = buf;
		}
		xmlNs =
			(xmlNsPtr) xml_parser_ns_add((WsXmlNodeH) xmlRoot, uri,
					prefix);
	}
	return (WsXmlNsH) xmlNs;
}

char *xml_parser_ns_query(WsXmlNsH ns, int what)
{
	xmlNsPtr xmlNs = (xmlNsPtr) ns;
	char *ptr = NULL;

	switch (what) {
	case XML_NS_URI:
		ptr = (char *) xmlNs->href;
		break;
	case XML_NS_PREFIX:
		ptr = (char *) xmlNs->prefix;
		break;
	default:
		assert(what == XML_NS_URI);
		break;
	}
	return ptr;
}


WsXmlNsH
xml_parser_ns_add(WsXmlNodeH node, const char *uri, const char *prefix)
{
	xmlNsPtr xmlNs = NULL;
	if (node && uri) {
		if ((xmlNs =
					(xmlNsPtr) xml_parser_ns_find(node, uri, NULL, 0,
						0)) != NULL) {
			if (xmlNs->prefix != NULL) {
				xmlFree((char *) xmlNs->prefix);
				xmlNs->prefix = NULL;
			}

			if (prefix != NULL) {
				xmlNs->prefix = xmlStrdup(BAD_CAST prefix);
			}
		} else {
			xmlNs =
				xmlNewNs((xmlNodePtr) node, BAD_CAST uri,
						BAD_CAST prefix);
		}
	}
	return (WsXmlNsH) xmlNs;
}


int xml_parser_ns_remove(WsXmlNodeH node, const char *nsUri)
{
	int retVal = -1;

	if (node && nsUri) {
		xmlNodePtr xmlNode = (xmlNodePtr) node;
		xmlNsPtr xmlNs = xmlNode->nsDef;
		xmlNsPtr prevNs = NULL;

		while (xmlNs != NULL) {
			if ((xmlStrEqual(xmlNs->href, BAD_CAST nsUri))) {
				break;
			}
			prevNs = xmlNs;
			xmlNs = xmlNs->next;
		}

		if (xmlNs != NULL) {
			retVal = 0;
			if (prevNs == NULL)
				xmlNode->nsDef = xmlNs->next;
			else
				prevNs->next = xmlNs->next;
			xmlFreeNs(xmlNs);
		} else
			retVal = 1;
	}
	return retVal;
}



WsXmlNsH xml_parser_ns_get(WsXmlNodeH node, int which)
{
	xmlNodePtr xmlNode = (xmlNodePtr) node;
	xmlNsPtr xmlNs = NULL;

	if (which >= 0) {
		int count = 0;
		xmlNs = xmlNode->nsDef;
		while (xmlNs != NULL) {
			if (which == count)
				break;
			count++;
			xmlNs = xmlNs->next;
		}
	} else {
		assert(which >= 0);
	}
	return (WsXmlNsH) xmlNs;
}


static
int get_ns_count_at_node(xmlNodePtr xmlNode)
{
	int count = 0;
	xmlNsPtr xmlNs = xmlNode->nsDef;

	while (xmlNs != NULL) {
		count++;
		xmlNs = xmlNs->next;
	}
	return count;
}


int xml_parser_get_count(WsXmlNodeH node, int what, int bWalkUpTree)
{
	int count = 0;
	xmlNodePtr xmlNode;
	xmlAttrPtr xmlAttr;

	switch (what) {
	case XML_COUNT_NODE:
		xmlNode = ((xmlNodePtr) node)->children;
		while (xmlNode) {
			if (xmlNode->type == XML_ELEMENT_NODE)
				count++;
			xmlNode = xmlNode->next;
		}
		break;
	case XML_COUNT_ATTR:
		xmlAttr = ((xmlNodePtr) node)->properties;
		while (xmlAttr) {
			count++;
			xmlAttr = xmlAttr->next;
		}
		break;
	case XML_COUNT_NS:
		xmlNode = (xmlNodePtr) node;
		while (xmlNode != NULL) {
			count += get_ns_count_at_node(xmlNode);
			if (!bWalkUpTree)
				break;
			xmlNode = xmlNode->parent;
		}
		break;
	default:
		assert(what == XML_COUNT_NODE || what == XML_COUNT_ATTR ||
				what == XML_COUNT_NS);
		break;
	}

	return count;
}

static xmlNodePtr
make_new_xml_node(xmlNodePtr base,
		const char *uri, const char *name, const char *value, int xmlescape)
{
	xmlNodePtr newNode = NULL;
	xmlNsPtr ns = NULL;
	if (uri == NULL ||
			(ns =
			 (xmlNsPtr) xml_parser_ns_find((WsXmlNodeH) base, uri, NULL, 1,
				 1)) != NULL) {
		if ((newNode = xmlNewNode(ns, BAD_CAST name)) != NULL) {
			if (value != NULL){		
				if (xmlescape == 1)
					xmlNodeAddContent(newNode, BAD_CAST value);
				else
					xmlNodeSetContent(newNode, BAD_CAST value);
			}
			newNode->_private = u_zalloc(sizeof(iWsNode));
		}
	}
	return newNode;
}






WsXmlNodeH
xml_parser_node_add(WsXmlNodeH base,
		int where,
		const char *nsUri,
		const char *localName, const char *value, int xmlescape)
{
	xmlNodePtr xmlBase = (xmlNodePtr) base;
	xmlNodePtr newNode =
		make_new_xml_node((where != XML_ELEMENT_NEXT &&
					where != XML_ELEMENT_PREV)
				? xmlBase : xmlBase->parent, nsUri,
				localName, value, xmlescape);
	if (newNode) {
		switch (where) {
		case XML_ELEMENT_NEXT:
			xmlAddNextSibling((xmlNodePtr) base, newNode);
			break;
		case XML_ELEMENT_PREV:
			xmlAddPrevSibling((xmlNodePtr) base, newNode);
			break;
		case XML_LAST_CHILD:
		default:
			xmlAddChild((xmlNodePtr) base, newNode);
			break;
		}
	}
	return (WsXmlNodeH) newNode;

}

int xml_parser_node_remove(WsXmlNodeH node)
{
	destroy_node_private_data(((xmlNodePtr) node)->_private);
	xmlUnlinkNode((xmlNodePtr) node);
	xmlFreeNode((xmlNodePtr) node);
	return 0;
}


WsXmlAttrH
xml_parser_attr_add(WsXmlNodeH node,
		const char *uri, const char *name, const char *value)
{
	xmlNodePtr xmlNode = (xmlNodePtr) node;
	xmlNsPtr xmlNs =
		(xmlNsPtr) xml_parser_ns_find(node, uri, NULL, 1, 1);
	xmlAttrPtr xmlAttr =
		(xmlAttrPtr) ws_xml_find_node_attr(node, uri, name);

	if (xmlAttr != NULL)
		ws_xml_remove_node_attr((WsXmlAttrH) xmlAttr);

	if (xmlNs == NULL)
		xmlAttr =
			xmlNewProp(xmlNode, BAD_CAST name, BAD_CAST value);
	else
		xmlAttr =
			xmlNewNsProp(xmlNode, xmlNs, BAD_CAST name,
					BAD_CAST value);

	if (xmlAttr != NULL) {
		if (xmlNs == NULL)
			xmlAttr->_private =
				xmlGetProp(xmlNode, BAD_CAST name);
		else
			xmlAttr->_private =
				xmlGetNsProp(xmlNode, BAD_CAST name,
						xmlNs->href);
	}

	return (WsXmlAttrH) xmlAttr;
}



int xml_parser_attr_remove(WsXmlAttrH attr)
{
	xmlAttrPtr xmlAttr = (xmlAttrPtr) attr;
	xmlNodePtr xmlNode = (xmlNodePtr) xmlAttr->parent;
	xmlAttrPtr xmlAttrPrev =
		(xmlNode->properties == xmlAttr) ? NULL : xmlNode->properties;

	while (xmlAttrPrev != NULL && xmlAttrPrev->next != xmlAttr) {
		xmlAttrPrev = xmlAttrPrev->next;
	}
	if (xmlAttrPrev != NULL)
		xmlAttrPrev->next = xmlAttr->next;
	else
		xmlNode->properties = xmlAttr->next;

	xmlNode->parent = NULL;
	xmlNode->next = NULL;

	destroy_attr_private_data((xmlAttrPtr) attr);
	xmlFreeProp((xmlAttrPtr) attr);

	return 0;
}

char *xml_parser_attr_query(WsXmlAttrH attr, int what)
{
	char *ptr = NULL;
	xmlAttrPtr xmlAttr = (xmlAttrPtr) attr;
	switch (what) {
	case XML_LOCAL_NAME:
		ptr = (char *) xmlAttr->name;
		break;
	case XML_NS_URI:
		if (xmlAttr->ns != NULL)
			ptr = (char *) xmlAttr->ns->href;
		break;
	case XML_NS_PREFIX:
		if (xmlAttr->ns != NULL)
			ptr = (char *) xmlAttr->ns->prefix;
		break;
	case XML_TEXT_VALUE:
		if (xmlAttr->_private == NULL) {
			if (xmlAttr->ns == NULL)
				xmlAttr->_private =
					xmlGetProp(xmlAttr->parent,
							xmlAttr->name);
			else
				xmlAttr->_private =
					xmlGetNsProp(xmlAttr->parent,
							xmlAttr->name,
							xmlAttr->ns->href);
		}
		ptr = (char *) xmlAttr->_private;
		break;
	default:
		assert(what == XML_LOCAL_NAME);
		break;
	}
	return ptr;
}



WsXmlAttrH xml_parser_attr_get(WsXmlNodeH node, int which)
{
	xmlNodePtr xmlNode = (xmlNodePtr) node;
	xmlAttrPtr xmlAttr = NULL;

	switch (which) {
	case XML_LAST_CHILD:
	default:
		if (which >= 0 || which == XML_LAST_CHILD) {
			int count = 0;
			xmlAttr = xmlNode->properties;

			while (xmlAttr) {
				if (which == XML_LAST_CHILD &&
						xmlAttr->next == NULL)
					break;

				if (which == count)
					break;

				count++;

				xmlAttr = xmlAttr->next;
			}
		} else {
			assert(which >= 0 || which == XML_LAST_CHILD);
		}
		break;
	}

	return (WsXmlAttrH) xmlAttr;
}



void xml_parser_element_dump(FILE * f, WsXmlDocH doc, WsXmlNodeH node)
{

	xmlNodePtr n = (xmlNodePtr) node;
	xmlDocPtr d = (xmlDocPtr) doc;
	xmlElemDump(f, d, n);
}

void xml_parser_doc_dump(FILE * f, WsXmlDocH doc)
{

	xmlDocPtr d = (xmlDocPtr) doc->parserDoc;
	xmlDocFormatDump(f, d, 1);
	return;
}

void xml_parser_doc_dump_memory(WsXmlDocH doc, char **buf, int *ptrSize)
{

	xmlDocPtr d = (xmlDocPtr) doc->parserDoc;
	xmlDocDumpFormatMemory(d, (xmlChar **) buf, ptrSize, 1);
	return;
}

static void
register_namespaces(xmlXPathContextPtr ctxt, WsXmlDocH doc,
		WsXmlNodeH node)
{
	xmlNsPtr *nsList, *cur;
	xmlDocPtr d = (xmlDocPtr) doc->parserDoc;


	nsList = xmlGetNsList(d, (xmlNodePtr) node);
	if (nsList == NULL) {
		return;
	}
	for (cur = nsList; *cur != NULL; cur++) {
		if (xmlXPathRegisterNs(ctxt, (*cur)->prefix, (*cur)->href)
				!= 0) {
			return;
		}
	}
	xmlFree(nsList);
}


int xml_parser_check_xpath(WsXmlDocH doc, const char *expression)
{
	xmlXPathObject *obj;
	xmlNodeSetPtr nodeset;
	xmlXPathContextPtr ctxt;
	xmlDocPtr d = (xmlDocPtr) doc->parserDoc;
	int retval = 0;

	ctxt = xmlXPathNewContext(d);
	if (ctxt == NULL) {
		error("failed while creating xpath context");
		return 0;
	}
	register_namespaces(ctxt, doc, xml_parser_get_root(doc));
	obj = xmlXPathEvalExpression(BAD_CAST expression, ctxt);
	if (obj) {
		nodeset = obj->nodesetval;
		if (nodeset && nodeset->nodeNr > 0) {
			int size = nodeset->nodeNr;
			int i;
			xmlNodePtr cur;
			for(i = 0; i < size; ++i) {
				if(nodeset->nodeTab[i]->type == XML_ELEMENT_NODE) {
					cur = nodeset->nodeTab[i];
					if(cur->ns) {
						fprintf(stdout, "= element node \"%s:%s\"\n",
								cur->ns->href, cur->name);
					} else {
						fprintf(stdout, "= element node \"%s\"\n",
								cur->name);
					}
				}
			}

			retval = 1;
		}
		xmlXPathFreeContext(ctxt);
		xmlXPathFreeObject(obj);
	} else {
		return 0;
	}

	return retval;
}



char *xml_parser_get_xpath_value(WsXmlDocH doc, const char *expression)
{
	//int i;
	char *result = NULL;
	xmlXPathObject *obj;
	xmlNodeSetPtr nodeset;
	xmlXPathContextPtr ctxt;
	xmlDocPtr d = (xmlDocPtr) doc->parserDoc;
	WsXmlNodeH body;

	ctxt = xmlXPathNewContext(d);
	if (ctxt == NULL) {
		error("failed while creating xpath context");
		return NULL;
	}
	body = ws_xml_get_soap_body(doc);
	register_namespaces(ctxt, doc, xml_parser_get_root(doc));
	if (ws_xml_get_child(body, 0, NULL, NULL)) {
		register_namespaces(ctxt, doc,
				ws_xml_get_child(body, 0, NULL, NULL));
	}

	obj = xmlXPathEvalExpression(BAD_CAST expression, ctxt);
	if (obj) {
		nodeset = obj->nodesetval;
		if (nodeset && nodeset->nodeNr > 0)
			result = (char *) xmlNodeListGetString(d,
					nodeset->
					nodeTab[0]->
					xmlChildrenNode,
					1);

		xmlXPathFreeContext(ctxt);
		xmlXPathFreeObject(obj);
	} else {
		return NULL;
	}

	return result;
}



void xml_parser_unlink_node(WsXmlNodeH node)
{
	xmlUnlinkNode((xmlNodePtr) node);
	xmlFreeNode((xmlNodePtr) node);
	return;
}

void xml_parser_node_set_lang(WsXmlNodeH node,  const char *lang)
{
	xmlNodeSetLang((xmlNodePtr) node, BAD_CAST lang);
}


void xml_parser_set_ns(WsXmlNodeH r, WsXmlNsH ns, const char *prefix)
{
	xmlSetNs((xmlNodePtr) r, (xmlNsPtr) ns);
}

void xml_parser_copy_node(WsXmlNodeH src, WsXmlNodeH dst)
{
	if (src && dst) {
		xmlNodePtr x = xmlDocCopyNode((xmlNodePtr) src,
				   ((xmlDocPtr) src)->doc, 1);
		if (x)
			xmlAddChild((xmlNodePtr) dst, x);
	}
}
