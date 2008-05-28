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
 * @author Liang Hou, Intel Corp.
 */

#include "u/libu.h"
#include "wsman-xml.h"
#include "wsman-names.h"
#include "wsman-filter.h"

filter_t * filter_initialize(void)
{
	filter_t *filter = u_zalloc(sizeof(filter_t));
	return filter;
}


static int filter_set(filter_t *filter, const char *dialect, const char *query, epr_t *epr, hash_t *selectors,
	const int assocType, const char *assocClass, const char *resultClass, const char *role,
	const char *resultRole, char **resultProp, const int propNum)
{
	int i = 0;

	if(dialect == NULL) {
		filter->dialect = u_strdup(WSM_XPATH_FILTER_DIALECT);
	} else {
		filter->dialect = u_strdup(dialect);
	}

	if (query) {
		filter->query = u_strdup(query);
	} else if(epr != 0) {
		filter->epr = epr_copy(epr);
		filter->assocType = assocType;
		if(assocClass)
			filter->assocClass = u_strdup(assocClass);
		if(resultClass)
			filter->resultClass = u_strdup(resultClass);
		if(role)
			filter->role = u_strdup(role);
		if(resultRole)
			filter->resultRole = u_strdup(resultRole);
		if(resultProp && propNum) {
			filter->resultProp = u_malloc(propNum*sizeof(char *));
			filter->PropNum = propNum;
			while(i < propNum) {
				filter->resultProp[i] = u_strdup(resultProp[i]);
				i++;
			}
		}
	} else if(selectors) {
		hnode_t        *hn;
		hscan_t         hs;
		Selector *p;
		selector_entry *entry;
		filter->selectorset.count = hash_count(selectors);
		filter->selectorset.selectors = u_malloc(sizeof(Selector)*
			filter->selectorset.count);

		p = filter->selectorset.selectors;
		hash_scan_begin(&hs, selectors);
		while ((hn = hash_scan_next(&hs))) {
			p->name = u_strdup((char *)hnode_getkey(hn));
			entry = (selector_entry *)hnode_get(hn);
			if(entry->type == 1) {
				p->type = 1;
				p->value = (char *)epr_copy(entry->entry.eprp);
				debug("key = %s value=%p(nested epr)",
					(char *) hnode_getkey(hn), p->value);
			} else {
				p->type = 0;
				p->value = u_strdup(entry->entry.text);
				debug("key = %s value=%s",
					(char *) hnode_getkey(hn), p->value);
			}
			p++;
		}
	}
	else
		goto cleanup;
	return 0;
cleanup:
	return 1;
}

filter_t *filter_create(const char *dialect, const char *query, epr_t *epr, hash_t *selectors,
	const int assocType, const char *assocClass, const char *resultClass, const char *role,
	const char *resultRole, char **resultProp, const int propNum)
{
	int ret = 0;
	filter_t *filter = filter_initialize();
	if (filter == NULL)
		return NULL;
	ret = filter_set(filter, dialect, query, epr, selectors,
		    assocType, assocClass, resultClass, role, resultRole, resultProp, propNum);
	if (ret == 1 ) {
		filter_destroy(filter);
		return NULL;
	} else {
		return filter;
	}
}

int filter_set_simple(filter_t *filter, const char *dialect, const char *query)
{
	return filter_set(filter, dialect, query, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, 0);
}

filter_t * filter_create_simple(const char *dialect, const char *query)
{
	return filter_create(dialect, query, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, 0);
}

int filter_set_assoc(filter_t *filter, epr_t *epr, const int assocType, const char *assocClass,
	const char *resultClass, const char *role, const char *resultRole, char **resultProp,
	const int propNum)
{
	return filter_set(filter, WSM_ASSOCIATION_FILTER_DIALECT, NULL, epr, NULL, assocType,
		assocClass, resultClass, role, resultRole, resultProp, propNum);
}


filter_t * filter_create_assoc(epr_t *epr, const int assocType, const char *assocClass,
	const char *resultClass, const char *role, const char *resultRole, char **resultProp,
	const int propNum)
{
	return filter_create(WSM_ASSOCIATION_FILTER_DIALECT, NULL, epr, NULL, assocType,
		assocClass, resultClass, role, resultRole, resultProp, propNum);
}

filter_t * filter_create_selector(hash_t *selectors)
{
	return filter_create(WSM_SELECTOR_FILTER_DIALECT, NULL, NULL, selectors, 0,
		NULL, NULL, NULL, NULL, NULL, 0);
}

int filter_add_selector(filter_t *filter, const char* key, const char *value)
{
	int i;
	Selector *entry;
	if(filter == NULL || key == NULL || value == NULL)
		return 0;
	entry = filter->selectorset.selectors;
	for(i = 0; i < filter->selectorset.count; i++) {
		if(strcmp(key, entry[i].name) == 0)
			return -1;
	}
	entry = u_realloc(entry, (filter->selectorset.count+1) * sizeof(Selector));
	if(entry == NULL) return -1;

	entry[filter->selectorset.count].type = 0;
	entry[filter->selectorset.count].name = u_strdup(key);
	entry[filter->selectorset.count].value = u_strdup(value);
	filter->selectorset.selectors = entry;
	filter->selectorset.count++;

	return 0;

}

filter_t * filter_copy(filter_t *filter)
{
	filter_t *filter_cpy = NULL;
	Selector *p1;
	Selector *p2;
	int i = 0;
	if(filter == NULL)
		return NULL;
	filter_cpy = u_zalloc(sizeof(filter_t));
	if(filter_cpy == NULL)
		return NULL;

	if(filter->dialect)
		filter_cpy->dialect = u_strdup(filter->dialect);
	filter_cpy->assocType = filter->assocType;
	if(filter->epr)
		filter_cpy->epr = epr_copy(filter->epr);
	if(filter->query)
		filter_cpy->query = u_strdup(filter->query);

	filter_cpy->selectorset.count = filter->selectorset.count;
	filter_cpy->selectorset.selectors = u_malloc(sizeof(Selector) *
		filter->selectorset.count);
	p1 = filter->selectorset.selectors;
	p2 = filter_cpy->selectorset.selectors;
	for(i = 0; i < filter_cpy->selectorset.count; i++) {
		p2->name = u_strdup(p1->name);
		p2->type = p1->type;
		if(p1->type == 0)
			p2->value = u_strdup(p1->value);
		else
			p2->value = (char *)epr_copy((epr_t*)p1->value);
		p1++;
		p2++;
	}

	if(filter->assocClass)
		filter_cpy->assocClass = u_strdup(filter->assocClass);
	if(filter->resultClass)
		filter_cpy->resultClass = u_strdup(filter->resultClass);
	if(filter->resultRole)
		filter_cpy->resultRole = u_strdup(filter->resultRole);
	if(filter->resultProp) {
		int i = 0;
		filter_cpy->resultProp = u_malloc(filter->PropNum*sizeof(char *));
		filter_cpy->PropNum = filter->PropNum;
		while(i < filter->PropNum) {
			filter_cpy->resultProp[i] = u_strdup(filter->resultProp[i]);
			i++;
		}
	}
	return filter_cpy;
}

void filter_destroy(filter_t *filter)
{
	Selector *p;
	int i;
	if(filter == NULL)
		return;
	if(filter->assocClass)
		u_free(filter->assocClass);
	if(filter->dialect)
		u_free(filter->dialect);
	if(filter->query)
		u_free(filter->query);
	if(filter->epr)
		epr_destroy(filter->epr);

	p = filter->selectorset.selectors;
	for(i = 0; i< filter->selectorset.count; i++) {
		u_free(p->name);
		if(p->type == 0)
			u_free(p->value);
		else
			epr_destroy((epr_t*)p->value);
		p++;
	}
	u_free(filter->selectorset.selectors);

	if(filter->resultClass)
		u_free(filter->resultClass);
	if(filter->resultProp) {
		int i = 0;
		while(i < filter->PropNum) {
			u_free(filter->resultProp[i]);
			i++;
		}
		u_free(filter->resultProp);
	}
	if(filter->resultRole)
		u_free(filter->resultRole);
	if(filter->role)
		u_free(filter->role);
	u_free(filter);
}

int filter_serialize(WsXmlNodeH node, filter_t *filter)
{
	int r = 0;
	WsXmlNodeH filter_node = NULL;
	WsXmlNodeH instance_node = NULL;
	if(filter->query) {
		filter_node = ws_xml_add_child(node, XML_NS_WS_MAN, WSM_FILTER, filter->query);
	} else if(filter->epr) {
		filter_node = ws_xml_add_child(node, XML_NS_WS_MAN, WSM_FILTER, NULL);
		if(filter->assocType == 0)
			instance_node = ws_xml_add_child(filter_node, XML_NS_CIM_BINDING, WSMB_ASSOCIATED_INSTANCES, NULL);
		else
			instance_node = ws_xml_add_child(filter_node, XML_NS_CIM_BINDING, WSMB_ASSOCIATION_INSTANCES, NULL);
		r = epr_serialize(instance_node, XML_NS_CIM_BINDING, WSMB_OBJECT, filter->epr, 1);

		if(r)
			return r;
		if(filter->assocClass)
			ws_xml_add_child(instance_node, XML_NS_CIM_BINDING, WSMB_ASSOCIATION_CLASS_NAME,
			filter->assocClass);
		if(filter->role)
			ws_xml_add_child(instance_node, XML_NS_CIM_BINDING, WSMB_ROLE, filter->role);
		if(filter->resultClass)
			ws_xml_add_child(instance_node, XML_NS_CIM_BINDING, WSMB_RESULT_CLASS_NAME,
			filter->resultClass);
		if(filter->resultRole)
			ws_xml_add_child(instance_node, XML_NS_CIM_BINDING, WSMB_RESULT_ROLE, filter->resultRole);
		if(filter->resultProp) {
			int i = 0;
			while(i < filter->PropNum) {
				ws_xml_add_child(instance_node, XML_NS_CIM_BINDING, WSMB_INCLUDE_RESULT_PROPERTY,
					filter->resultProp[i]);
				i++;
			}
		}

	} else if(filter->selectorset.count) {
		int i = 0;
		filter_node = ws_xml_add_child(node, XML_NS_WS_MAN, WSM_FILTER, NULL);
		node = ws_xml_add_child(filter_node, XML_NS_WS_MAN, WSM_SELECTOR_SET, NULL);

		while (i < filter->selectorset.count) {
			if(filter->selectorset.selectors[i].type == 0) {
				instance_node = ws_xml_add_child(node, XML_NS_WS_MAN, WSM_SELECTOR,
					filter->selectorset.selectors[i].value);
				ws_xml_add_node_attr(instance_node, NULL, WSM_NAME, filter->selectorset.selectors[i].name);
			}
			else {
				epr_serialize(node, NULL, NULL, (epr_t *)filter->selectorset.selectors[i].value, 1);
			}
			i++;
		}
	}
	else {
		return -1;
	}
	if(filter->dialect)
		ws_xml_add_node_attr(filter_node, NULL, WSM_DIALECT, filter->dialect);
	return r;

}

filter_t * filter_deserialize(WsXmlNodeH node)
{
	char *dialect = NULL;
	int properNum = 0;
	int i = 0;
	WsXmlAttrH attr = NULL;
	filter_t *filter = NULL;
	WsXmlNodeH instance_node = NULL;
	WsXmlNodeH entry_node = NULL;
	WsXmlNodeH filter_node = ws_xml_get_child(node, 0, XML_NS_WS_MAN, WSM_FILTER);
	if(filter_node == NULL) return NULL;
	filter = u_zalloc(sizeof(filter_t));
	dialect = ws_xml_find_attr_value(filter_node, NULL, WSM_DIALECT);
	if(dialect)
		filter->dialect = u_strdup(dialect);
	else{
		attr = ws_xml_get_node_attr(filter_node, 0);
		if(attr) {
			filter->dialect = u_strdup(ws_xml_get_attr_value(attr));
		}
		else
			filter->dialect = u_strdup(WSM_XPATH_FILTER_DIALECT);
	}
	if(strcmp(filter->dialect , WSM_ASSOCIATION_FILTER_DIALECT) == 0) {
		instance_node = ws_xml_get_child(filter_node, 0, XML_NS_CIM_BINDING, WSMB_ASSOCIATED_INSTANCES);
		if(instance_node) {
			filter->assocType = 0;
		}
		else {
			instance_node = ws_xml_get_child(filter_node, 0, XML_NS_CIM_BINDING, WSMB_ASSOCIATION_INSTANCES);
			if(instance_node) {
				filter->assocType = 1;
			}
			else
				goto CLEANUP;
		}
		filter->epr = epr_deserialize(instance_node, XML_NS_CIM_BINDING, WSMB_OBJECT, 1);
		entry_node = ws_xml_get_child(instance_node, 0, XML_NS_CIM_BINDING, WSMB_ASSOCIATION_CLASS_NAME);
		if(entry_node)
			filter->assocClass = u_strdup(ws_xml_get_node_text(entry_node));
		entry_node = ws_xml_get_child(instance_node, 0, XML_NS_CIM_BINDING, WSMB_ROLE);
		if(entry_node)
			filter->role = u_strdup(ws_xml_get_node_text(entry_node));
		entry_node = ws_xml_get_child(instance_node, 0, XML_NS_CIM_BINDING, WSMB_RESULT_CLASS_NAME);
		if(entry_node)
			filter->resultClass = u_strdup(ws_xml_get_node_text(entry_node));
		entry_node = ws_xml_get_child(instance_node, 0, XML_NS_CIM_BINDING, WSMB_RESULT_ROLE);
		if(entry_node)
			filter->resultRole = u_strdup(ws_xml_get_node_text(entry_node));
		properNum = ws_xml_get_child_count(instance_node) - 4;
		filter->resultProp = u_zalloc(properNum * sizeof(char*));
		while(i < properNum) {
			filter_node = ws_xml_get_child(instance_node, i, XML_NS_CIM_BINDING, WSMB_INCLUDE_RESULT_PROPERTY);
			if(filter_node == NULL)
				break;
			filter->resultProp[i] = u_strdup(ws_xml_get_node_text(filter_node));
			i++;
		}
		filter->PropNum = i;
	}
	else if(strcmp(filter->dialect, WSM_SELECTOR_FILTER_DIALECT) == 0) {
		filter_node = ws_xml_get_child(filter_node, 0, XML_NS_WS_MAN, WSM_SELECTOR_SET);
		if(filter_node == NULL)
			goto CLEANUP;
		filter->selectorset.count = ws_xml_get_child_count(filter_node);
		filter->selectorset.selectors = u_malloc(sizeof(Selector) * filter->selectorset.count );
		while(i < filter->selectorset.count) {
			entry_node = ws_xml_get_child(filter_node, i, XML_NS_WS_MAN, WSM_SELECTOR);
			if(entry_node == NULL) break;
			attr = ws_xml_find_node_attr(entry_node, NULL, WSM_NAME);
			if(attr) {
				filter->selectorset.selectors[i].name = u_strdup(ws_xml_get_attr_value(attr));
			}
			instance_node = ws_xml_get_child(entry_node, 0, XML_NS_ADDRESSING, WSA_EPR);
			if(instance_node) {
				filter->selectorset.selectors[i].type = 1;
				filter->selectorset.selectors[i].value = (char *)epr_deserialize(instance_node, NULL, NULL, 1);
			}
			else {
				filter->selectorset.selectors[i].type = 0;
				filter->selectorset.selectors[i].value = u_strdup(ws_xml_get_node_text(entry_node));
			}
			i++;
		}
	}
	else
		filter->query = u_strdup(ws_xml_get_node_text(filter_node));

	return filter;

CLEANUP:
	filter_destroy(filter);
	return NULL;
}
