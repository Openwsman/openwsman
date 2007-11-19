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

#ifndef WSMAN_EPR_H_
#define WSMAN_EPR_H_

#include <wsman-xml-serializer.h>

typedef struct {
	XML_TYPE_STR value;
	XML_NODE_ATTR *attrs;
} Selector;



typedef struct {
	XML_TYPE_DYN_ARRAY selectors;
} SelectorSet;



typedef struct {
	XML_TYPE_STR uri;
	SelectorSet selectorset;
} ReferenceParameters;


typedef struct {
	XML_TYPE_STR type;
	XML_TYPE_STR address;
	ReferenceParameters refparams;
} epr_t;



typedef int (*selector_callback ) (void *, const char*, const char*);

epr_t *wsman_get_epr(WsContextH cntx, WsXmlNodeH node,
	const char *epr_node_name, const char *ns);

void wsman_epr_selector_cb(epr_t *epr, selector_callback cb, void *cb_data);
char *wsman_epr_selector_by_name(epr_t *epr, const char* name);


#endif
