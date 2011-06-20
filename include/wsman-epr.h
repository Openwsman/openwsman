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

#ifndef WSMAN_EPR_H_
#define WSMAN_EPR_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#include "wsman-types.h"
#include "u/hash.h"

typedef struct {
	char *value; //string or nestes epr_t
	char *name;
	int type; // type = 0, value is text; Or else, value is a nested epr_t
} Selector;

typedef struct {
	unsigned int count;
	Selector *selectors;
} SelectorSet;



typedef struct {
	char * uri;
	SelectorSet selectorset;
} ReferenceParameters;


typedef struct {
	char * address;
	ReferenceParameters refparams;
	void * additionalParams;
} epr_t;

typedef struct {
	int type;
	union{
		char *text;
		epr_t *eprp;
	} entry;
} selector_entry;

/* support for array values, all represented with the same key */	
typedef struct {
	char * key;
	void * data;
	int arraycount;
} methodarglist_t;

typedef int (*selector_callback ) (void *, const char*, const char*);

void wsman_epr_selector_cb(epr_t *epr, selector_callback cb,
		void *cb_data);

void wsman_selectorset_cb(SelectorSet *selectorset, selector_callback cb,
		void *cb_data);

char *wsman_epr_selector_by_name(epr_t *epr, const char* name);

int epr_selector_count(epr_t *epr);

char *epr_get_resource_uri(epr_t *epr);

 /**
 * Create an epr_t structure
 * @param uri Resource URI string
 * @param selectors A hash which contains pairs of name:selector_entry
 * @param address If it is NULL, "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous"
                             will be set
 * @return created epr_t address
 */
epr_t *epr_create(const char *uri, hash_t * selectors,
		const char *address);
/**
 * Create an epr_t from a string
 * @param str A string in a format of "Resource_uri?name1=value1&name2=value2"
 * @return created epr_t address
 */
 epr_t *epr_from_string(const char* str);

/**
 * Add a new text selector to an exsiting epr
 * @param epr Point of epr_t
 * @param name Name of selector
 * @param selector Point of seletor_entry
 * @return 0 for sucess, others for failure
 */
int epr_add_selector_text(epr_t *epr, const char *name, const char *text);

/**
 * Add a new epr selector to an exsiting epr
 * @param epr Point of epr_t
 * @param name Name of selector
 * @param selector Point of seletor_entry
 * @return 0 for sucess, others for failure
 */
int epr_add_selector_epr(epr_t *epr, const char *name, epr_t *added_epr);

/**
 * Delete an selector from an epr by name
 * @param epr Point of epr_t
 * @param name Name of selector name
 * @return 0 for sucess, others for failure
 */
int epr_delete_selector(epr_t *epr, const char *name);

 /**
 * Destroy an epr_t structure
 * @param epr An epr point
 * @return void
 */
void epr_destroy(epr_t *epr);

 /**
 * Create a new epr_t from an original epr_t
 * @param epr An epr point
 * @return created epr_t address
 */
epr_t *epr_copy(epr_t *epr);

/**
 * Compare two epr_ts
 * @param epr1
 * @param epr2
 * @return 0 for equality, others mean inequality
 */
 int epr_cmp(epr_t *epr1, epr_t *epr2);

/**
 * Turn an epr_t structure to an XML snippet.
 * @param node XML node which will contain epr XML snippet
 * @param ns Namespace of EPR wrapper name
 * @param epr_node_name EPR wrapper name
 * @param epr A point of epr_t
 * @param embedded It means a complete epr snippet if embedded is 1. Or else, it is
                                 a snippet used in a soap header.
 * @return 0 for success, others for failure
 */
int epr_serialize(WsXmlNodeH node, const char *ns,
		const char *epr_node_name, epr_t *epr, int embedded);

/**
 * Form an epr_t structure from an XML snippet.
 * @param node XML node which contains epr XML snippet
 * @param ns Namespace of EPR wrapper name
 * @param epr_node_name EPR wrapper name
 * @param embedded It means a complete epr snippet if embedded is 1. Or else, it is
                                 a snippet used in a soap header.
 * @return New created epr_t point
 */
epr_t *epr_deserialize(WsXmlNodeH node, const char *ns,
		const char *epr_node_name, int embedded);

/**
 * Get cim namespace selector from a SelectorSet
 * @param selectorset
 * @return CIM namespace if there is
 */
char *get_cimnamespace_from_selectorset(SelectorSet *selectorset);

char *epr_to_txt(epr_t *epr, const char *ns, const char*epr_node_name);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
