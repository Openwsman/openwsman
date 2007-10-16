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
        epr = (epr_t *) ws_deserialize(cntx,
                                        node,
                                        epr_t_TypeInfo,
                                        epr_node_name,
                                        ns, NULL, 0, 0);
	return epr;
}


char *wsman_epr_selector_by_name(epr_t *epr, const char* name)
{
	int i;
	char *value = NULL;
	XML_NODE_ATTR *a;
	Selector *ss =
		(Selector *) epr->refparams.selectorset.selectors.data;
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
	Selector *ss =
		(Selector *) epr->refparams.selectorset.selectors.data;
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


