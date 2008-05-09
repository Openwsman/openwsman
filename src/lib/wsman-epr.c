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
 * @author Anas Nashif, Intel Corp.
 * @author Liang Hou, Intel Corp.
 */


#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-client-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"

#include "wsman-faults.h"
#include "wsman-soap-envelope.h"

#include "wsman-epr.h"

char *wsman_epr_selector_by_name(epr_t *epr, const char* name)
{
	int i;
	char *value = NULL;
	Selector *ss = (Selector *) epr->refparams.selectorset.selectors;
	if (ss == NULL) {
			debug("epr->refparams.selectors.data == NULL\n");
		return NULL;
	}
	for (i = 0; i < epr->refparams.selectorset.count; i++) {
		Selector *s;
		s = ss + i;
		if (strcmp(s->name, name) == 0 && s->type == 0) {
				value =  u_strdup(s->value);
				break;
		}
	}
	return value;
}


void wsman_epr_selector_cb(epr_t *epr, selector_callback cb, void *cb_data)
{
	int i;
	Selector *ss = (Selector *) epr->refparams.selectorset.selectors;
	if (ss == NULL) {
		debug("epr->refparams.selectors == NULL\n");
		return;
	}
	for (i = 0; i < epr->refparams.selectorset.count; i++) {
		Selector *s;
		s = ss + i;
		cb(cb_data, s->name, s->value);
	}
}

void wsman_selectorset_cb(SelectorSet *selectorset, selector_callback cb, void *cb_data)
{
	int i;
	Selector *ss = selectorset->selectors;
	if (ss == NULL) {
		debug("epr->refparams.selectors == NULL");
		return;
	}
	for (i = 0; i < selectorset->count; i++) {
		Selector *s;
		s = ss + i;
		cb(cb_data, s->name, s->value);
	}
}

epr_t *epr_create(const char *uri, hash_t * selectors, const char *address)
{
	epr_t *epr = NULL;
	epr = u_malloc(sizeof(epr_t));
	if (address == NULL)
		epr->address = u_strdup(WSA_TO_ANONYMOUS);
	else
		epr->address = u_strdup(address);

	epr->refparams.uri = u_strdup(uri);

	if (selectors) {
		hnode_t        *hn;
		hscan_t         hs;
		Selector *p;
		selector_entry *entry;
		epr->refparams.selectorset.count = hash_count(selectors);
		epr->refparams.selectorset.selectors = u_malloc(sizeof(Selector)*
			epr->refparams.selectorset.count);

		p = epr->refparams.selectorset.selectors;
		hash_scan_begin(&hs, selectors);
		while ((hn = hash_scan_next(&hs))) {
			p->name = u_strdup((char *)hnode_getkey(hn));
			entry = (selector_entry *)hnode_get(hn);
			if(entry->type == 0) {
				p->type = 0;
				p->value = u_strdup(entry->entry.text);
				debug("key = %s value=%s",
					(char *) hnode_getkey(hn), p->value);
			}
			else {
				p->type = 1;
				p->value = (char *)epr_copy(entry->entry.eprp);
				debug("key = %s value=%p(nested epr)",
					(char *) hnode_getkey(hn), p->value);
			}
			p++;
		}
	} else {
		epr->refparams.selectorset.count  = 0;
		epr->refparams.selectorset.selectors  = NULL;
	}
	return epr;
}

 epr_t *epr_from_string(const char* str)
 {
 	char *p;
	char *uri;
	hash_t *selectors;
	hash_t *selectors_new;
	hnode_t        *hn;
	hscan_t         hs;
	selector_entry *entry;
	epr_t *epr;

	p = strchr(str, '?');
	uri = u_strndup(str, p - str);
	selectors = u_parse_query(p + 1);
	selectors_new = hash_create2(HASHCOUNT_T_MAX, 0, 0);

	hash_scan_begin(&hs, selectors);
	while ((hn = hash_scan_next(&hs))) {
		entry = u_malloc(sizeof(selector_entry));
		entry->type = 0;
		entry->entry.text = (char *)hnode_get(hn);
		hash_alloc_insert(selectors_new, hnode_getkey(hn), entry);
	}

	epr = epr_create(uri, selectors_new, NULL);
	hash_free(selectors_new);
	hash_free(selectors);
	u_free(uri);
	return epr;
 }

static int epr_add_selector(epr_t *epr, const char *name, selector_entry *selector)
 {
 	int i;
 	Selector *p;
	if(epr == NULL) return 0;
 	p = epr->refparams.selectorset.selectors;
	for(i = 0; i< epr->refparams.selectorset.count; i++) {
		if(p->name && ( strcmp(name, p->name) == 0 ) ) {
			return -1;
		}
		p++;
	}
	p = epr->refparams.selectorset.selectors;
	p = u_realloc(p, (epr->refparams.selectorset.count+1) * sizeof(Selector));
	if(p == NULL) return -1;
	p[epr->refparams.selectorset.count].name = u_strdup(name);
	p[epr->refparams.selectorset.count].type = selector->type;
	if(selector->type == 0) {
		if (selector->entry.text) {
			p[epr->refparams.selectorset.count].value = u_strdup(selector->entry.text);
		}
	} else {
		p[epr->refparams.selectorset.count].value = (char *)epr_copy(selector->entry.eprp);
	}

	epr->refparams.selectorset.selectors = p;
	epr->refparams.selectorset.count++;
	return 0;
 }

int epr_selector_count(epr_t *epr) {
	if(epr == NULL) return 0;
	return epr->refparams.selectorset.count;
}

int epr_add_selector_text(epr_t *epr, const char *name, const char *text)
{
	int r;
	selector_entry *entry;
	entry = u_malloc(sizeof(selector_entry));
	entry->type = 0;
	entry->entry.text = (char *)text;
	r = epr_add_selector(epr, name, entry);
	u_free(entry);
	return r;
}

int epr_add_selector_epr(epr_t *epr, const char *name, epr_t *added_epr)
{
	int r;
	selector_entry *entry;
	entry = u_malloc(sizeof(selector_entry));
	entry->type = 1;
	entry->entry.eprp = added_epr;
	r = epr_add_selector(epr, name, entry);
	u_free(entry);
	return r;
}

int epr_delete_selector(epr_t *epr, const char *name)
{
	int i,k;
	int count;
	Selector *selectors;
	if(epr == NULL || name == NULL) return 0;
	count = epr->refparams.selectorset.count;
	selectors = epr->refparams.selectorset.selectors;
	for(i =0; i < count; i++) {
		if(strcmp(name, selectors[i].name) == 0)
			break;
	}
	if(i == count) return -1;

	u_free(selectors[i].name);
	if(selectors[i].type == 0) {
		u_free(selectors[i].value);
	}
	else {
		epr_destroy((epr_t *)selectors[i].value);
	}

	for(k = i; k < count-1; k++) {
		memcpy(&selectors[k], &selectors[k+1], sizeof(Selector));
	}

	epr->refparams.selectorset.selectors = u_realloc(selectors, (count-1)*sizeof(Selector));
	epr->refparams.selectorset.count--;

	return 0;
}

void epr_destroy(epr_t *epr)
{
	int i;
	Selector *p;
	if(epr == NULL) return;
	u_free(epr->address);
	u_free(epr->refparams.uri);
	p = epr->refparams.selectorset.selectors;
	for(i = 0; i< epr->refparams.selectorset.count; i++) {
		u_free(p->name);
		if(p->type == 0)
			u_free(p->value);
		else
			epr_destroy((epr_t*)p->value);
		p++;
	}

	u_free(epr->refparams.selectorset.selectors);
	u_free(epr);

}

epr_t *epr_copy(epr_t *epr)
{
	int i;
	Selector *p1;
	Selector *p2;
	epr_t *cpy_epr = NULL;
	if(epr == NULL)
		return cpy_epr;

	cpy_epr = u_malloc(sizeof(epr_t));
	if (epr && epr->address)
		cpy_epr->address = u_strdup(epr->address);

	cpy_epr->refparams.uri = u_strdup(epr->refparams.uri);
	cpy_epr->refparams.selectorset.count = epr->refparams.selectorset.count;
	cpy_epr->refparams.selectorset.selectors = u_malloc(sizeof(Selector)*
			epr->refparams.selectorset.count);

	p1 = epr->refparams.selectorset.selectors;
	p2 = cpy_epr->refparams.selectorset.selectors;
	for(i = 0; i < epr->refparams.selectorset.count; i++) {
		p2->name = u_strdup(p1->name);
		p2->type = p1->type;
		if(p1->type == 0)
			p2->value = u_strdup(p1->value);
		else
			p2->value = (char *)epr_copy((epr_t*)p1->value);
		p1++;
		p2++;
	}
	return cpy_epr;
}

 int epr_cmp(epr_t *epr1, epr_t *epr2)
 {
 	int i, j;
 	int matches = 0;
	Selector *p1;
	Selector *p2;
	assert(epr1 != NULL && epr2 != NULL);
	//if(strcmp(epr1->address, epr2->address)) return 1;

	if(strcmp(epr1->refparams.uri, epr2->refparams.uri)) return 1;
	if(epr1->refparams.selectorset.count != epr2->refparams.selectorset.count)
		return 1;
	p1 = epr1->refparams.selectorset.selectors;
	for(i = 0; i < epr1->refparams.selectorset.count; i++) {
		p2 = epr1->refparams.selectorset.selectors;
		for(j = 0; j < epr2->refparams.selectorset.count; j++, p2++) {
			if(strcmp(p1->name, p2->name))
				continue;
			if(p1->type != p2->type)
				continue;
			if(p1->type == 0) {
				if(strcmp(p1->value, p2->value))
					continue;
			} else {
				if (epr_cmp((epr_t*)p1->value, (epr_t*)p2->value) == 1) {
					continue;
				}
			}
			matches++;
		}
		p1++;
	}

	if (matches == epr1->refparams.selectorset.count)
		return 0;
	else
		return 1;
}

char *epr_to_txt(epr_t *epr, const char *ns, const char*epr_node_name)
{
	char *buf = NULL;
	int len;
	WsXmlDocH doc2;
	WsXmlDocH doc = ws_xml_create_doc(ns, epr_node_name);
	WsXmlNodeH rootNode = ws_xml_get_doc_root(doc);
	epr_serialize(rootNode, NULL, NULL, epr, 1);
	doc2 = ws_xml_create_doc_by_import( rootNode);
	ws_xml_dump_memory_node_tree(ws_xml_get_doc_root(doc), &buf, &len);
	ws_xml_destroy_doc(doc);;
	ws_xml_destroy_doc(doc2);
	return buf;
}


char *epr_get_resource_uri(epr_t *epr) {
	if (epr)
		return epr->refparams.uri;
	else
		return NULL;
}

int epr_serialize(WsXmlNodeH node, const char *ns,
		const char *epr_node_name, epr_t *epr, int embedded)
{
	int i;
	WsXmlNodeH eprnode = NULL;
	WsXmlNodeH refparamnode = NULL;
	WsXmlNodeH selectorsetnode = NULL;
	Selector *p = NULL;
	if(epr == NULL) return 0;

	if(epr_node_name) {
		eprnode = ws_xml_add_child(node, ns, epr_node_name, NULL);
	}
	else
		eprnode = node;
	if(embedded)
		ws_xml_add_child(eprnode, XML_NS_ADDRESSING, WSA_ADDRESS, epr->address);
	else
		ws_xml_add_child(eprnode, XML_NS_ADDRESSING, WSA_TO, epr->address);
	if(embedded)
		refparamnode = ws_xml_add_child(eprnode, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS, NULL);
	else
		refparamnode = node;

	ws_xml_add_child(refparamnode, XML_NS_WS_MAN, WSM_RESOURCE_URI, epr->refparams.uri);
	selectorsetnode = ws_xml_add_child(refparamnode, XML_NS_WS_MAN, WSM_SELECTOR_SET, NULL);

	p = epr->refparams.selectorset.selectors;
	for(i = 0; i < epr->refparams.selectorset.count; i++) {
		WsXmlNodeH temp = NULL;
		if(p->type == 0)
			temp = ws_xml_add_child(selectorsetnode, XML_NS_WS_MAN, WSM_SELECTOR, p->value);
		else {
			temp = ws_xml_add_child(selectorsetnode, XML_NS_WS_MAN, WSM_SELECTOR, NULL);
			epr_serialize(temp, XML_NS_ADDRESSING, WSA_EPR, (epr_t *)p->value, 1);
		}
		ws_xml_add_node_attr(temp, NULL, WSM_NAME, p->name);
		p++;
	}
	return 0;
}

epr_t *epr_deserialize(WsXmlNodeH node, const char *ns,
		const char *epr_node_name, int embedded)
{
	int i;
	epr_t *epr = u_malloc(sizeof(epr_t));

	WsXmlNodeH eprnode = NULL;
	WsXmlNodeH refparamnode = NULL;
	WsXmlNodeH temp = NULL;
	WsXmlNodeH selectorsetnode = NULL;
	WsXmlAttrH attr = NULL;
	Selector *p = NULL;

	if(epr_node_name) {
		eprnode = ws_xml_get_child(node, 0, ns, epr_node_name);
		if(eprnode == NULL)
			goto CLEANUP;
	} else {
		eprnode = node;
	}

	if(embedded) {
		temp = ws_xml_get_child(eprnode, 0, XML_NS_ADDRESSING, WSA_ADDRESS);
	} else {
		temp = ws_xml_get_child(eprnode, 0, XML_NS_ADDRESSING, WSA_TO);
	}

	if(temp == NULL)
		goto CLEANUP;
	epr->address = u_strdup(ws_xml_get_node_text(temp));

	if(embedded) {
		refparamnode = ws_xml_get_child(eprnode, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);
	} else {
		refparamnode = node;
	}

	if(refparamnode == NULL)
		goto CLEANUP;

	temp = ws_xml_get_child(refparamnode, 0, XML_NS_WS_MAN, WSM_RESOURCE_URI);
	if(temp == NULL)
		goto CLEANUP;

	epr->refparams.uri = u_strdup(ws_xml_get_node_text(temp));

	selectorsetnode = ws_xml_get_child(refparamnode, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);
	epr->refparams.selectorset.count = ws_xml_get_child_count(selectorsetnode);
	epr->refparams.selectorset.selectors = u_malloc(epr->refparams.selectorset.count *
		 sizeof(Selector));

	p = epr->refparams.selectorset.selectors;
	for(i = 0; i < epr->refparams.selectorset.count; i++) {
		temp = ws_xml_get_child(selectorsetnode, i, XML_NS_WS_MAN, WSM_SELECTOR);
		attr = ws_xml_find_node_attr(temp, NULL, "Name");
		if(attr) {
			p->name = u_strdup(ws_xml_get_attr_value(attr));
		}
		if(ws_xml_get_child(temp, 0, XML_NS_ADDRESSING, WSA_EPR)) {
			p->type = 1;
			p->value = (char *)epr_deserialize(temp, XML_NS_ADDRESSING, WSA_EPR, 1);
		} else {
			p->type = 0;
			p->value = u_strdup(ws_xml_get_node_text(temp));
		}
		p++;
	}

	return epr;
CLEANUP:
	u_free(epr);
	return NULL;

}

char *get_cimnamespace_from_selectorset(SelectorSet *selectorset)
{
	int i = 0;
	while(i < selectorset->count) {
		if(strcmp(selectorset->selectors[i].name, CIM_NAMESPACE_SELECTOR) == 0)
			return selectorset->selectors[i].value;
		i++;
	}
	return NULL;
}

