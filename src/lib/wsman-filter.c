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


filter_t * filter_create(const char *dialect, const char *query, epr_t *epr, const int assocType,
	const char *assocClass, const char *resultClass, const char *role, const char *resultRole, 
	const char **resultProp, const int propNum)
{
	int i = 0;
	filter_t *filter = u_zalloc(sizeof(filter_t));
	if(filter == NULL) return filter;

	if(dialect == NULL)
		filter->dialect = u_strdup(WSM_XPATH_FILTER_DIALECT);
	else
		filter->dialect = u_strdup(dialect);

	if(query)
		filter->query = u_strdup(query);
	else {
		if(epr == NULL) goto cleanup;
		
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
	}
	return filter;
cleanup:
	filter_destroy(filter);
	return NULL;
}

filter_t * filter_copy(filter_t *filter)
{
	filter_t *filter_cpy = NULL;
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
	if(filter->query)
		filter_node = ws_xml_add_child(node, XML_NS_WS_MAN, WSM_FILTER, filter->query);
	else {
		filter_node = ws_xml_add_child(node, XML_NS_WS_MAN, WSM_FILTER, NULL);
		if(filter->epr == NULL)
			return -1;
		if(filter->assocType == 0)
			instance_node = ws_xml_add_child(filter_node, XML_NS_CIM_BINDING, WSMB_ASSOCIATED_INSTANCES, NULL);
		else
			instance_node = ws_xml_add_child(filter_node, XML_NS_CIM_BINDING, WSMB_ASSOCIATION_INSTANCES, NULL);
		r = epr_serialize(instance_node, XML_NS_CIM_BINDING, WSMB_OBJECT, filter->epr, 1);
		if(r) return r;
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
			filter->query = u_strdup(ws_xml_get_node_text(filter_node));
	}
	if(filter->query)
		return filter;
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
	return filter;
}
