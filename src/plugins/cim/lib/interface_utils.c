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
 * @author Haihao Xiang
 * @author Anas Nashif 
 */


#include "interface_utils.h"

GList * add_key(WsContextH cntx, GList *keys, char *name)
{
	wsman_debug( WSMAN_DEBUG_LEVEL_DEBUG, "Adding Key: %s", name );
	WsSelectorInfo *sel= malloc(sizeof(WsSelectorInfo));
	if (!sel) 
	{
		wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "malloc failed");
		return NULL;
	}
	sel->key = name;
	if ( (sel->val = wsman_get_selector(cntx, NULL, name, 0)) == NULL)
	{
		wsman_debug(WSMAN_DEBUG_LEVEL_ERROR, "NULL Key");
		return NULL;
	}
	keys = g_list_append(keys, sel);
	return keys;
}

GList *create_key_list(WsContextH cntx, WsSelector *selectors) 
{
	GList *keys = NULL;
	int i;
	for(i = 0; selectors && selectors[i].name != NULL; i++)
	{
		if ( (keys = add_key(cntx, keys, selectors[i].name)) == NULL )
		{
			return NULL;
		}
	}		
	return keys;
}






