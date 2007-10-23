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
 * @author Liang Hou
 */
#include "u/libu.h"
#include "wsman-cimxmllistener-path.h"

static list_t *cimxml_listener_path_list = NULL;

char * create_cimxml_listener_path(char *uuid)
{
	char path[128];
	snprintf(path, 128, "/wsman/cimindicationlistener/%s", uuid);
	return u_strdup(path);
}

int add_cimxml_listener_path(char *uuid)
{
	if(cimxml_listener_path_list == NULL)
		cimxml_listener_path_list = list_create(-1);
	char *path = create_cimxml_listener_path(uuid);
	lnode_t *node = lnode_create(path);
	list_append(cimxml_listener_path_list, node);
	return 0;
}

list_t * get_cimxml_listener_path(void)
{
	return cimxml_listener_path_list;
}

int delete_cimxml_listener_path(char *uuid)
{
	char *path = NULL;
	if(cimxml_listener_path_list == NULL) return 0;
	lnode_t *node = list_first(cimxml_listener_path_list);
	char *service_path = create_cimxml_listener_path(uuid);
	while(node) {
		path= (char*)node->list_data;
		if(strstr(path, service_path)) {
			break;
		}
		node = list_next(cimxml_listener_path_list, node);
	}
	u_free(service_path);
	if(node == NULL) return -1;
	list_delete(cimxml_listener_path_list, node);
	u_free(path);
	lnode_destroy(node);
	return 0;
}

