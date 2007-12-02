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

static hash_t *cimxml_listener_path = NULL;
static pthread_mutex_t cimxml_listener_path_lock;

char * create_cimxml_listener_path(char *uuid)
{
		char path[128];
		snprintf(path, 128, "/wsman/cimindicationlistener/%s", uuid);
		return u_strdup(path);
}

int add_cimxml_listener_path(char *uuid)
{
		pthread_mutex_lock(&cimxml_listener_path_lock);
		if(cimxml_listener_path == NULL)
				cimxml_listener_path = hash_create(HASHCOUNT_T_MAX,0,0);
		char *path = create_cimxml_listener_path(uuid);
		hash_alloc_insert(cimxml_listener_path, uuid, path);
		pthread_mutex_unlock(&cimxml_listener_path_lock);
		return 0;
}

char * get_cimxml_listener_path(char *uuid)
{
		pthread_mutex_lock(&cimxml_listener_path_lock);
		hnode_t *hn = hash_lookup(cimxml_listener_path, uuid);
		char *service_path = hnode_get(hn);
		pthread_mutex_unlock(&cimxml_listener_path_lock);
		return service_path;
}

void get_cimxml_listener_paths(char ***paths, int *num)
{
		hscan_t hs;
		hnode_t *hn;
		*paths = NULL;
		*num = 0;
		pthread_mutex_lock(&cimxml_listener_path_lock);
		if(cimxml_listener_path == NULL) {
				pthread_mutex_unlock(&cimxml_listener_path_lock);
				return;
		}
		int count = hash_count(cimxml_listener_path);
		if(count == 0) {
				pthread_mutex_unlock(&cimxml_listener_path_lock);
				return;
		}
		char **servicepaths = u_malloc(count*sizeof(char*));
		hash_scan_begin(&hs, cimxml_listener_path);
		int i = 0;
		while ((hn = hash_scan_next(&hs))) {
				servicepaths[i] = u_strdup((char*) hnode_get(hn));
				i++;
		}
		*paths = servicepaths;
		*num = count;
		pthread_mutex_unlock(&cimxml_listener_path_lock);
}

int delete_cimxml_listener_path(char *uuid)
{
		pthread_mutex_lock(&cimxml_listener_path_lock);
		if(cimxml_listener_path == NULL) {
				pthread_mutex_unlock(&cimxml_listener_path_lock);
				return 0;
		}
		hnode_t *hn = hash_lookup(cimxml_listener_path, uuid);
		if(hn) {
				u_free(hnode_get(hn));
				hash_delete_free(cimxml_listener_path, hn);
		}
		pthread_mutex_unlock(&cimxml_listener_path_lock);
		return 0;
}

