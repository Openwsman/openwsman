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
 */


#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-client-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-faults.h"
#include "wsman-soap-envelope.h"

#include "wsman-epr.h"

SER_TYPEINFO_STRING_ATTR;
SER_START_ITEMS(SelectorSet)
SER_NS_DYN_ARRAY(XML_NS_WS_MAN, WSM_SELECTOR, 0, 10,
		string_attr), SER_END_ITEMS(SelectorSet);


SER_START_ITEMS(ReferenceParameters)
SER_NS_STR(XML_NS_WS_MAN, WSM_RESOURCE_URI, 1),
	SER_NS_STRUCT(XML_NS_WS_MAN, WSM_SELECTOR_SET, 1, SelectorSet),
	SER_END_ITEMS(ReferenceParameters);


SER_START_ITEMS(epr_t)
SER_NS_STR(XML_NS_ADDRESSING, WSA_ADDRESS, 1),
SER_NS_STRUCT(XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS, 1,
			ReferenceParameters), SER_END_ITEMS(epr_t);



epr_t  *wsman_get_epr(WsContextH cntx, WsXmlNodeH node,
		const char *epr_node_name, const char *ns)
{
	epr_t *epr;
    	epr = (epr_t *) ws_deserialize(cntx->serializercntx, node, epr_t_TypeInfo,
                                        epr_node_name,
                                        ns, NULL, 0, 0);
	return epr;
}


char *wsman_epr_selector_by_name(epr_t *epr, const char* name)
{
	int i;
	char *value = NULL;
	XML_NODE_ATTR *a;
	Selector *ss = (Selector *) epr->refparams.selectorset.selectors.data;
	if (ss == NULL) {
			debug("epr->refparams.selectors.data == NULL\n");
		return NULL;
	}
	for (i = 0; i < epr->refparams.selectorset.selectors.count; i++) {
		Selector *s;
		s = ss + i;
		a = s->attrs;
		while (a) {
			if (strcmp(a->name, WSM_NAME) == 0 && strcmp(a->value, name) == 0 ) {
				value =  u_strdup(s->value);
				break;
			}
			a = a->next;
		}
	}
	return value;
}


void wsman_epr_selector_cb(epr_t *epr, selector_callback cb, void *cb_data)
{
	int i;
	XML_NODE_ATTR *a;
	Selector *ss = (Selector *) epr->refparams.selectorset.selectors.data;
	if (ss == NULL) {
			debug("epr->refparams.selectors.data == NULL\n");
		return;
	}
	for (i = 0; i < epr->refparams.selectorset.selectors.count; i++) {
		Selector *s;
		s = ss + i;
		a = s->attrs;
		while (a) {
			if (strcmp(a->name, WSM_NAME) == 0 ) {
				cb(cb_data, a->value, s->value );
				break;
			}
			a = a->next;
		}
	}
}

epr_t *epr_create(const char *uri, hash_t * selectors, const char *address)
{
	epr_t *epr = NULL;
	epr = u_malloc(sizeof(epr_t));
	if(address == NULL)
		epr->address = u_strdup(WSA_TO_ANONYMOUS);
	else
		epr->address = u_strdup(address);

	epr->refparams.uri = u_strdup(uri);
	
	if (selectors) {
		hnode_t        *hn;
		hscan_t         hs;
		
		epr->refparams.selectorset.selectors.count = hash_count(selectors);
		epr->refparams.selectorset.selectors.data = u_malloc(sizeof(SelectorPlus)*
			epr->refparams.selectorset.selectors.count);
		
		SelectorPlus *p = epr->refparams.selectorset.selectors.data;
		hash_scan_begin(&hs, selectors);
		while ((hn = hash_scan_next(&hs))) {
			p->attrs = u_malloc(sizeof(XML_NODE_ATTR));
			p->attrs->next = NULL;
			p->attrs->ns = NULL;
			p->attrs->name = u_strdup("Name");
			p->attrs->value = u_strdup((char *)hnode_getkey(hn));
			selector_entry *entry = (selector_entry *)hnode_get(hn);
			if(entry->type == 0) {
				p->type = 0;
				p->value = u_strdup(entry->entry.text);
				debug("key = %s value=%s",
					(char *) hnode_getkey(hn), p->value);
			}
			else {
				p->type = 1;
				p->value = (XML_TYPE_STR)epr_copy(entry->entry.eprp);
				debug("key = %s value=%p(nested epr)",
					(char *) hnode_getkey(hn), p->value);
			}			
			p++;
		}
	}
	return epr;
}

void epr_destroy(epr_t *epr)
{
	int i;
	if(epr == NULL) return;
	u_free(epr->address);
	u_free(epr->refparams.uri);
	
	SelectorPlus *p = epr->refparams.selectorset.selectors.data;
	for(i = 0; i< epr->refparams.selectorset.selectors.count; i++) {
		u_free(p->attrs->name);
		u_free(p->attrs->value);
		u_free(p->attrs);
		if(p->type == 0)
			u_free(p->value);
		else
			epr_destroy((epr_t*)p->value);
		p++;
	}
	
	u_free(epr->refparams.selectorset.selectors.data);
	u_free(epr);
	
}

epr_t *epr_copy(epr_t *epr)
{
	int i;
	epr_t *cpy_epr = NULL;
	if(epr == NULL) return cpy_epr;

	cpy_epr = u_malloc(sizeof(epr_t));
	cpy_epr->address = u_strdup(epr->address);
	cpy_epr->refparams.uri = u_strdup(epr->refparams.uri);
	cpy_epr->refparams.selectorset.selectors.count = epr->refparams.selectorset.selectors.count;
	cpy_epr->refparams.selectorset.selectors.data = u_malloc(sizeof(SelectorPlus)*
			epr->refparams.selectorset.selectors.count);

	SelectorPlus *p1 = epr->refparams.selectorset.selectors.data;
	SelectorPlus *p2 = cpy_epr->refparams.selectorset.selectors.data;
	for(i = 0; i < epr->refparams.selectorset.selectors.count; i++) {
		p2->attrs = u_malloc(sizeof(XML_NODE_ATTR));
		p2->attrs->name = u_strdup(p1->attrs->name);
		p2->attrs->next = NULL;
		p2->attrs->ns = NULL;
		p2->attrs->value = u_strdup(p1->attrs->value);
		p2->type = p1->type;
		if(p1->type == 0)
			p2->value = u_strdup(p1->value);
		else
			p2->value = (XML_TYPE_STR)epr_copy((epr_t*)p1->value);
		p1++;
		p2++;
	}
	return cpy_epr;
}

int epr_serialize(WsXmlNodeH node, epr_t *epr, int embedded)
{
	if(epr == NULL) return 0;
	int i;

	WsXmlNodeH eprnode = NULL;
	WsXmlNodeH refparamnode = NULL;

	if(embedded) {
		eprnode = ws_xml_add_child(node, XML_NS_ADDRESSING, WSA_EPR, NULL);
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
	WsXmlNodeH selectorsetnode = ws_xml_add_child(refparamnode, XML_NS_WS_MAN, WSM_SELECTOR_SET, NULL);

	SelectorPlus *p = epr->refparams.selectorset.selectors.data;
	for(i = 0; i < epr->refparams.selectorset.selectors.count; i++) {
		WsXmlNodeH temp = NULL;
		if(p->type == 0)
			temp = ws_xml_add_child(selectorsetnode, XML_NS_WS_MAN, WSM_SELECTOR, p->value);
		else {
			temp = ws_xml_add_child(selectorsetnode, XML_NS_WS_MAN, WSM_SELECTOR, NULL);
			epr_serialize(temp, (epr_t *)p->value, 1);
		}
		ws_xml_add_node_attr(temp, p->attrs->ns, p->attrs->name, p->attrs->value);
		p++;
	}
	return 0;
}

epr_t *epr_deserialize(WsXmlNodeH node, int embedded)
{
	int i;
	epr_t *epr = u_malloc(sizeof(epr_t));
	
	WsXmlNodeH eprnode = NULL;
	WsXmlNodeH refparamnode = NULL;
	WsXmlNodeH temp = NULL;

	if(embedded) {
		eprnode = ws_xml_get_child(node, 0, XML_NS_ADDRESSING, WSA_EPR);
		if(eprnode == NULL) 
			goto CLEANUP;
	}
	else
		eprnode = node;

	if(embedded) {
		temp = ws_xml_get_child(eprnode, 0, XML_NS_ADDRESSING, WSA_ADDRESS);
	}
	else
		temp = ws_xml_get_child(eprnode, 0, XML_NS_ADDRESSING, WSA_TO);
	if(temp == NULL)
		goto CLEANUP;
	epr->address = u_strdup(ws_xml_get_node_text(temp));

	if(embedded) {
		refparamnode = ws_xml_get_child(eprnode, 0, XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);
	}
	else
		refparamnode = node;
	if(refparamnode == NULL)
		goto CLEANUP;
	temp = ws_xml_get_child(refparamnode, 0, XML_NS_WS_MAN, WSM_RESOURCE_URI);
	if(temp == NULL)
		goto CLEANUP;
	epr->refparams.uri = u_strdup(ws_xml_get_node_text(temp));

	WsXmlNodeH selectorsetnode = ws_xml_get_child(refparamnode, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);
	epr->refparams.selectorset.selectors.count = ws_xml_get_child_count(selectorsetnode);
	epr->refparams.selectorset.selectors.data = u_malloc(epr->refparams.selectorset.selectors.count *
		 sizeof(SelectorPlus));
	
	SelectorPlus *p = epr->refparams.selectorset.selectors.data;
	for(i = 0; i < epr->refparams.selectorset.selectors.count; i++) {
		temp = ws_xml_get_child(selectorsetnode, i, XML_NS_WS_MAN, WSM_SELECTOR);
		WsXmlAttrH attr = ws_xml_find_node_attr(temp, NULL, "Name");
		if(attr) {
			p->attrs = (XML_NODE_ATTR *)u_malloc(sizeof(XML_NODE_ATTR));
			p->attrs->name = u_strdup("Name");
			p->attrs->next = NULL;
			p->attrs->ns = NULL;
			p->attrs->value = u_strdup(ws_xml_get_attr_value(attr));
		}
		if(ws_xml_get_child(temp, 0, XML_NS_ADDRESSING, WSA_EPR)) {
			p->type = 1;
			p->value = (XML_TYPE_STR)epr_deserialize(temp, 1);
		}
		else {
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

