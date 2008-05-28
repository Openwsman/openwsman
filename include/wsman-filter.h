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

#ifndef _WSMAN_FILTER_H_
#define _WSMAN_FILTER_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#include "wsman-types.h"
#include "wsman-epr.h"

struct __filter_t{
	char *dialect;
	char *query;
	epr_t * epr; //for "http://schemas.dmtf.org/wsman/cimbinding/associationFilter"
	SelectorSet selectorset; //for "http://schemas.dmtf.org/wbem/wsman/1/wsman/SelectorFilter" dialect
	int assocType; // 0 for associated instance query, 1 for association instance query
	char *assocClass;
	char *resultClass;
	char *role;
	char *resultRole;
	char **resultProp;
	int PropNum;
};

typedef struct __filter_t filter_t;


filter_t * filter_initialize(void);

filter_t * filter_create(const char *dialect, const char *query, epr_t *epr, hash_t *selectors,
	const int assocType, const char *assocClass, const char *resultClass, const char *role,
	const char *resultRole, char **resultProp, const int propNum);

int filter_set_simple(filter_t *filter, const char *dialect, const char *query);

int filter_set_assoc(filter_t *filter, epr_t *epr, const int assocType, const char *assocClass,
    const char *resultClass, const char *role, const char *resultRole, char **resultProp,
    const int propNum);

/* void filter_set_selector(filter_t *filter, hash_t *selectors, const char *cimnamespace); */


/**
 * Create a new filter_t with a simple query string
 * @param dialect Filter dialect string
 * @param query Query sting
 * @ return created filter_t strucrture point
 */
filter_t * filter_create_simple(const char *dialect, const char *query);

/**
 * Create a new filter_t with assciation query
 * @param epr EPR structure, Identifies the source object for the association query and is required
 * @param assocType Association query type: 0 for associated instances, 1 for association instances
 * @param assocClass Represents the name of a CIM association class.  Optional
 * @param resultClass Represents the name of a CIM class.  Optional.
 * @param role Represents the name of a key reference property of a CIM association class.  Optional
 * @param resultRole Represents the name of a key reference property of a CIM association class.  Optional
 * @param resultProp Represents the name of one or more properties of a CIM class. Optional
 * @param propNum Number of resultProp
 * @return created filter_t strucrture point
 */
filter_t * filter_create_assoc(epr_t *epr, const int assocType, const char *assocClass,
	const char *resultClass, const char *role, const char *resultRole, char **resultProp,
	const int propNum);

/**
 * Create a new filter_t with SelectorSet query
 * @param selectors A hash which contains pairs of name:selector_entry
 * @param cimnamespace Specify cim namespace if it is needed
 * @return created filter_t strucrture point
 */
filter_t * filter_create_selector(hash_t *selectors);

/**
 * Add more text selectors to an existing SelectorSet filter
 * @param filter
 * @param key
 * @param value
 *@ return 0 for success, others mean failure
 */
int filter_add_selector(filter_t *filter, const char* key, const char *value);

/**
 * Create a new filter_t from an original filter_t
 * @param filter
 * @return new created filter_t structure point
 */
filter_t * filter_copy(filter_t *filter);

/**
 * Destroy a filter_t
 * @param filter
 * @return void
 */
void filter_destroy(filter_t *filter);

/**
 * Turn filter_t structure into an XML snippet
 * @param node Node which contains filter XML snippet
 * @param filter A point to filter_t structure
 * @return 0 for success, others mean failure
 */
int filter_serialize(WsXmlNodeH node, filter_t *filter);

/**
 * Form filter_t structure from an XML snippet
 * @param node Node which contains filter XML snippet
 * @return filter_t  A point to a malloced filter_t structure
 */
filter_t * filter_deserialize(WsXmlNodeH node);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
