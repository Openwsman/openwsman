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

char *wsman_epr_selector_by_name(const epr_t *epr, const char* name)
{
	char *value = NULL;
	key_value_t *ss = epr->refparams.selectorset.selectors;
	if (ss == NULL) {
			debug("epr->refparams.selectorset.selectors == NULL\n");
		return NULL;
	}
	for (unsigned int i = 0; i < epr->refparams.selectorset.count; i++) {
		key_value_t *s;
		s = ss + i;
		if (strcmp(s->key, name) == 0 && s->type == 0) {
				value =  u_strdup(s->v.text);
				break;
		}
	}
	return value;
}


void wsman_epr_selector_cb(const epr_t *epr, selector_callback cb, void *cb_data)
{
	key_value_t *ss = epr->refparams.selectorset.selectors;
	if (ss == NULL) {
		debug("epr->refparams.selectorset.selectors == NULL\n");
		return;
	}
	for (unsigned int i = 0; i < epr->refparams.selectorset.count; i++) {
		key_value_t *s;
		s = ss + i;
		cb(cb_data, s);
	}
}

void wsman_selectorset_cb(SelectorSet *selectorset, selector_callback cb, void *cb_data)
{
	key_value_t *ss = selectorset->selectors;
	if (ss == NULL) {
		debug("epr->refparams.selectors == NULL");
		return;
	}
	for (unsigned int i = 0; i < selectorset->count; i++) {
		key_value_t *s;
		s = ss + i;
		cb(cb_data, s);
	}
}

epr_t *epr_create(const char *uri, hash_t *selectors, const char *address)
{
	epr_t *epr = NULL;

	epr = u_malloc(sizeof(epr_t));
	if (epr == NULL) {
		return NULL;
	}

	if (address == NULL)
		epr->address = u_strdup(WSA_TO_ANONYMOUS);
	else
		epr->address = u_strdup(address);

	if (!epr->address) {
		goto CLEANUP;
	}

	epr->refparams.uri = u_strdup(uri);
	if (!epr->refparams.uri) {
		goto CLEANUP;
	}

	if (selectors) {
		hnode_t        *hn;
		hscan_t         hs;
		key_value_t *p;
		key_value_t *entry;
		epr->refparams.selectorset.count = hash_count(selectors);
		epr->refparams.selectorset.selectors = u_malloc(sizeof(key_value_t)*
			epr->refparams.selectorset.count);
		if (epr->refparams.selectorset.selectors != NULL)
		{
			p = epr->refparams.selectorset.selectors;
			hash_scan_begin(&hs, selectors);
			while ((hn = hash_scan_next(&hs))) {
				entry = (key_value_t *)hnode_get(hn);
				if (entry->type == 0) {
					key_value_create((char *)hnode_getkey(hn), entry->v.text, NULL, p);
					debug("key=%s value=%s", p->key, p->v.text);
				}
				else {
					key_value_create((char *)hnode_getkey(hn), NULL, entry->v.epr, p);
					debug("key=%s value=%p(nested epr)", p->key, p->v.epr);
				}
				p++;
			}
		}
	} else {
		epr->refparams.selectorset.count  = 0;
		epr->refparams.selectorset.selectors  = NULL;
	}

	return epr;

CLEANUP:
	u_free(epr->address);
	u_free(epr);
	return NULL;

}

epr_t *epr_from_string(const char* str)
{
	char *p;
	char *uri;
	hash_t *selectors = NULL;
	hash_t *selectors_new = NULL;
	hnode_t        *hn;
	hscan_t         hs;
	key_value_t *entry;
	epr_t *epr = NULL;

	p = strchr(str, '?');
	if (p) {
		uri = u_strndup(str, p - str);
		selectors = u_parse_query(p + 1);
		if (selectors) {
			selectors_new = hash_create2(HASHCOUNT_T_MAX, 0, 0);
			if (selectors_new) {
				hash_scan_begin(&hs, selectors);
				while ((hn = hash_scan_next(&hs))) {
					entry = key_value_create(NULL, (char *)hnode_get(hn), NULL, NULL);
					hash_alloc_insert(selectors_new, hnode_getkey(hn), entry);
				}
			}
		}
	} else {
		uri = u_strdup(str);
	}

	if (uri)
		epr = epr_create(uri, selectors_new, NULL);

	if (selectors_new)
		hash_free(selectors_new);
	if (selectors)
		hash_free(selectors);

	u_free(uri);
	return epr;
 }

static int epr_add_selector(epr_t *epr, const char *name, const char *text, epr_t *added_epr)
{
	key_value_t *p, *new_p;
	if (epr == NULL) return 0;
	p = epr->refparams.selectorset.selectors;
	for(unsigned int i = 0; i< epr->refparams.selectorset.count; i++) {
		if(p->key && ( strcmp(name, p->key) == 0 ) ) {
			return -1;
		}
		p++;
	}
	p = epr->refparams.selectorset.selectors;
	p = u_realloc(p, (epr->refparams.selectorset.count+1) * sizeof(key_value_t));
	if (p == NULL) return -1;
	epr->refparams.selectorset.selectors = p;
	new_p = key_value_create(name, text, added_epr, &(p[epr->refparams.selectorset.count]));
	if (new_p == NULL) {
		return -1;
	}
	epr->refparams.selectorset.count++;
	return 0;
 }

int epr_selector_count(const epr_t *epr) {
	if(epr == NULL) return 0;
	return epr->refparams.selectorset.count;
}

int epr_add_selector_text(epr_t *epr, const char *name, const char *text)
{
	return epr_add_selector(epr, name, text, NULL);
}

int epr_add_selector_epr(epr_t *epr, const char *name, epr_t *added_epr)
{
	return epr_add_selector(epr, name, NULL, added_epr);
}

int epr_delete_selector(epr_t *epr, const char *name)
{
	int i,k;
	int count;
	key_value_t *selectors;
	if(epr == NULL || name == NULL) return 0;
	count = epr->refparams.selectorset.count;
	selectors = epr->refparams.selectorset.selectors;
	for(i =0; i < count; i++) {
		if(strcmp(name, selectors[i].key) == 0)
			break;
	}
	if(i == count) return -1;

        key_value_destroy(&selectors[i], 1);

	for(k = i; k < count-1; k++) {
		memcpy(&selectors[k], &selectors[k+1], sizeof(key_value_t));
	}

	key_value_t *temp = u_realloc(selectors, (count-1)*sizeof(key_value_t));
	if (!temp)
		return -1;
	epr->refparams.selectorset.selectors = temp;
	epr->refparams.selectorset.count--;

	return 0;
}

void epr_destroy(epr_t *epr)
{
	key_value_t *p;
	if(epr == NULL) return;
	u_free(epr->address);
	u_free(epr->refparams.uri);
	p = epr->refparams.selectorset.selectors;
	for(unsigned int i = 0; i< epr->refparams.selectorset.count; i++) {
          key_value_destroy(p, 1);
		p++;
	}

	u_free(epr->refparams.selectorset.selectors);
	u_free(epr);

}

epr_t *epr_copy(const epr_t *epr)
{
	key_value_t *p1;
	key_value_t *p2;
	epr_t *cpy_epr = NULL;
	if(epr == NULL)
		return cpy_epr;

	cpy_epr = u_malloc(sizeof(epr_t));
	if (cpy_epr == NULL) {
		return NULL;
	}

	if (epr && epr->address)
		cpy_epr->address = u_strdup(epr->address);

	cpy_epr->refparams.uri = u_strdup(epr->refparams.uri);
	cpy_epr->refparams.selectorset.selectors = u_malloc(sizeof(key_value_t)*
			epr->refparams.selectorset.count);

	if (cpy_epr->refparams.selectorset.selectors) {
		cpy_epr->refparams.selectorset.count = epr->refparams.selectorset.count;
		p1 = epr->refparams.selectorset.selectors;
		p2 = cpy_epr->refparams.selectorset.selectors;
		for (unsigned int i = 0; i < epr->refparams.selectorset.count; i++) {
			key_value_copy(p1, p2);
			p1++;
			p2++;
		}
	} else {
		cpy_epr->refparams.selectorset.count = 0;
	}
	return cpy_epr;
}

int epr_cmp(const epr_t *epr1, const epr_t *epr2)
{
	int matches = 0;
	key_value_t *p1;
	key_value_t *p2;
	assert(epr1 != NULL && epr2 != NULL);
	//if(strcmp(epr1->address, epr2->address)) return 1;

	if(strcmp(epr1->refparams.uri, epr2->refparams.uri)) return 1;
	if(epr1->refparams.selectorset.count != epr2->refparams.selectorset.count)
		return 1;
	p1 = epr1->refparams.selectorset.selectors;
	for(unsigned int i = 0; i < epr1->refparams.selectorset.count; i++) {
		p2 = epr1->refparams.selectorset.selectors;
		for(unsigned int j = 0; j < epr2->refparams.selectorset.count; j++, p2++) {
			if(strcmp(p1->key, p2->key))
				continue;
			if(p1->type != p2->type)
				continue;
			if(p1->type == 0) {
				if(strcmp(p1->v.text, p2->v.text))
					continue;
			} else {
				if (epr_cmp(p1->v.epr, p2->v.epr) == 1) {
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

char *epr_to_string(const epr_t *epr)
{
  int len;
  char *buf, *ptr;

  key_value_t *p = NULL;
  if (epr == NULL) return NULL;

  /* calculate buffer size */
  len = strlen(epr->refparams.uri);

  p = epr->refparams.selectorset.selectors;
  for(unsigned int i = 0; i < epr->refparams.selectorset.count; i++) {
    len += (strlen(p->key) + 1); /* (?|&)key */
    if (p->type == 0)
      len += (strlen(p->v.text) + 1); /* =value */
    else {
      char *value = epr_to_string(p->v.epr);
      if (value) {
        len += (strlen(value) + 1); /* =value */
        u_free(value);
      }
    }
    p++;
  }
  buf = u_malloc(len + 1);
  if (buf) {
	  strcpy(buf, epr->refparams.uri);
	  ptr = buf + strlen(buf);
	  p = epr->refparams.selectorset.selectors;
	  for (unsigned int i = 0; i < epr->refparams.selectorset.count; i++) {
		  if (i == 0)
			  *ptr++ = '?';
		  else
			  *ptr++ = '&';
		  strcpy(ptr, p->key);
		  ptr += strlen(p->key);
		  *ptr++ = '=';
		  if (p->type == 0) {
			  strcpy(ptr, p->v.text);
			  ptr += strlen(p->v.text);
		  } else {
			  char *value = epr_to_string(p->v.epr);
			  if (value) {
				  strcpy(ptr, value);
				  ptr += strlen(value);
				  u_free(value);
			  }
		  }
		  p++;
	  }
	  *ptr++ = 0;
  }
  return buf;
}


char *epr_to_txt(const epr_t *epr, const char *ns, const char*epr_node_name)
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


char *epr_get_resource_uri(const epr_t *epr) {
	if (epr)
		return epr->refparams.uri;
	else
		return NULL;
}

int epr_serialize(WsXmlNodeH node, const char *ns,
		const char *epr_node_name, const epr_t *epr, int embedded)
{
	WsXmlNodeH eprnode = NULL;
	WsXmlNodeH refparamnode = NULL;
	WsXmlNodeH selectorsetnode = NULL;
	key_value_t *p = NULL;
	if(epr == NULL) return 0;

	if(epr_node_name) {
		eprnode = ws_xml_add_child(node, ns, epr_node_name, NULL);
	}
	else
		eprnode = node;

	if (eprnode == NULL) {
		return 0;
	}

	if(embedded)
		ws_xml_add_child(eprnode, XML_NS_ADDRESSING, WSA_ADDRESS, epr->address);
	else
		ws_xml_add_child(eprnode, XML_NS_ADDRESSING, WSA_TO, epr->address);
	if(embedded)
		refparamnode = ws_xml_add_child(eprnode, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS, NULL);
	else
		refparamnode = node;

	if (refparamnode == NULL) {
		return 0;
	}
	ws_xml_add_child(refparamnode, XML_NS_WS_MAN, WSM_RESOURCE_URI, epr->refparams.uri);
	selectorsetnode = ws_xml_add_child(refparamnode, XML_NS_WS_MAN, WSM_SELECTOR_SET, NULL);
	if (selectorsetnode == NULL) {
		return 0;
	}
	p = epr->refparams.selectorset.selectors;
	for(unsigned int i = 0; i < epr->refparams.selectorset.count; i++) {
		WsXmlNodeH temp = NULL;
		if(p->type == 0)
			temp = ws_xml_add_child(selectorsetnode, XML_NS_WS_MAN, WSM_SELECTOR, p->v.text);
		else {
			temp = ws_xml_add_child(selectorsetnode, XML_NS_WS_MAN, WSM_SELECTOR, NULL);
			if (temp == NULL) {
				return 0;
			}
			epr_serialize(temp, XML_NS_ADDRESSING, WSA_EPR, p->v.epr, 1);
		}
		ws_xml_add_node_attr(temp, NULL, WSM_NAME, p->key);
		p++;
	}
	return 0;
}

epr_t *epr_deserialize(WsXmlNodeH node, const char *ns,
		const char *epr_node_name, int embedded)
{
	epr_t *epr = u_zalloc(sizeof(epr_t));
	if (epr == NULL) {
		goto CLEANUP;
	}

	WsXmlNodeH eprnode = NULL;
	WsXmlNodeH refparamnode = NULL;
	WsXmlNodeH temp = NULL;
	WsXmlNodeH selectorsetnode = NULL;
	WsXmlAttrH attr = NULL;
	key_value_t *p = NULL;

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

	epr->address = u_strdup(ws_xml_get_node_text_safe(temp));
	if(epr->address == NULL) {
		goto CLEANUP;
	}

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

	epr->refparams.uri = u_strdup(ws_xml_get_node_text_safe(temp));
	if (epr->refparams.uri == NULL) {
		goto CLEANUP;
	}

	selectorsetnode = ws_xml_get_child(refparamnode, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);
	epr->refparams.selectorset.count = ws_xml_get_child_count(selectorsetnode);
	epr->refparams.selectorset.selectors = u_malloc(epr->refparams.selectorset.count *
		 sizeof(key_value_t));

	if (epr->refparams.selectorset.selectors) {
		p = epr->refparams.selectorset.selectors;
		for (unsigned int i = 0; i < epr->refparams.selectorset.count; i++) {
			temp = ws_xml_get_child(selectorsetnode, i, XML_NS_WS_MAN, WSM_SELECTOR);
			attr = ws_xml_find_node_attr(temp, NULL, "Name");
			if (attr) {
				p->key = u_strdup(ws_xml_get_attr_value(attr));
			}
			if (ws_xml_get_child(temp, 0, XML_NS_ADDRESSING, WSA_EPR)) {
				p->type = 1;
				p->v.epr = epr_deserialize(temp, XML_NS_ADDRESSING, WSA_EPR, 1);
			} else {
				p->type = 0;
				p->v.text = u_strdup(ws_xml_get_node_text_safe(temp));
			}
			p++;
		}
	} else {
		epr->refparams.selectorset.count = 0;
	}

	return epr;
CLEANUP:
	if (epr) {
		u_free(epr->address);
		u_free(epr->refparams.uri);
	}
	u_free(epr);
	return NULL;

}

char *get_cimnamespace_from_selectorset(SelectorSet *selectorset)
{
	unsigned int i = 0;
	while(i < selectorset->count) {
		if(strcmp(selectorset->selectors[i].key, CIM_NAMESPACE_SELECTOR) == 0)
			return selectorset->selectors[i].v.text;
		i++;
	}
	return NULL;
}

