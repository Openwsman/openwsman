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
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#include "wsman-soap.h"
#include "wsman-names.h"
#include "wsman-types.h"
#include "wsman-xml-api.h"
#include "wsman-xml-binding.h"

int LocalSubscriptionOpInit (char * uri_repository, void *opaqueData);
int LocalSubscriptionOpFinalize(char * uri_repository, void *opaqueData);
int LocalSubscriptionOpGet(char * uri_repository, char * uuid, unsigned char **subscriptionDoc, int *len);
int LocalSubscriptionOpSearch(char * uri_repository, char * uuid);
int LocalSubscriptionOpLoad (char * uri_repository, list_t * subscription_list);
int LocalSubscriptionOpSave (char * uri_repository, char * uuid, unsigned char *subscriptionDoc);
int LocalSubscriptionOpUpdate(char * uri_repository, char * uuid, char *expire);
int LocalSubscriptionOpDelete (char * uri_repository, char * uuid);


struct __SubsRepositoryOpSet subscription_repository_op_set = {LocalSubscriptionOpInit, LocalSubscriptionOpFinalize, LocalSubscriptionOpLoad, LocalSubscriptionOpGet, LocalSubscriptionOpSearch, LocalSubscriptionOpSave, LocalSubscriptionOpUpdate, LocalSubscriptionOpDelete};

static int LocalSubscriptionInitFlag = 0;

SubsRepositoryOpSetH wsman_get_subsrepos_opset()
{
	return &subscription_repository_op_set;
}

int LocalSubscriptionOpInit (char * uri_repository, void *opaqueData)
{
	LocalSubscriptionInitFlag = 1;

	return 0;
}

int LocalSubscriptionOpFinalize(char * uri_repository, void *opaqueData)
{
	if(LocalSubscriptionInitFlag == 0) return -1;
	return 0;
}

int LocalSubscriptionOpGet(char * uri_repository, char * uuid, unsigned char  **subscriptionDoc, int *len)
{
	unsigned char block[512];
	unsigned char *buf = NULL;
	int count,m;
	int pre_count;
	count = m = 0;
	*subscriptionDoc = NULL;
	if(LocalSubscriptionInitFlag == 0) return -1;
	char *subs_path = u_strdup_printf ("%s/uuid:%s", uri_repository, uuid);
	FILE *fp = fopen(subs_path, "r");
	u_free(subs_path);
	if(fp == NULL) return -1;
	while(!feof(fp)) {
				memset(block, 0, 512);
				m = fread(block, 1, 511, fp);
				if(m > 0) {
					debug("read [%s] from file, len = %d",block, m);
					pre_count = count;
					count += m;
					debug("buf = %0x, count = %d", buf, count);
					buf = u_realloc(buf, count);
					memcpy(buf+pre_count, block, m);
				}
	}
	fclose(fp);
	*subscriptionDoc = buf;
	*len = count;
	return 0;
}

int LocalSubscriptionOpSearch(char * uri_repository, char * uuid)
{
	if(LocalSubscriptionInitFlag == 0) return -1;
	char *subs_path = u_strdup_printf ("%s/uuid:%s", uri_repository, uuid);
	FILE *fp = fopen(subs_path, "r");
	u_free(subs_path);
	if(fp == NULL) return -1;
	fclose(fp);
	return 0;
}

int LocalSubscriptionOpLoad (char * uri_repository, list_t * subscription_list)
{
	struct dirent **namelist;
	int n, m, count;
	int pre_count;
	char block[512];
	unsigned char *buf = NULL;
	if(LocalSubscriptionInitFlag == 0) return -1;
	if(subscription_list == NULL)
		return -1;
	if (0 > (n = scandir (uri_repository, &namelist, 0, alphasort)))
    	{
       	 return -1;
    	} else {
        	while (n--)
        	{
        		lnode_t *node;
			if(strstr(namelist[n]->d_name,"uuid") == NULL || strlen(namelist[n]->d_name) < 41) {
				u_free(namelist[n]);
				continue;
			}
			char *subs_path = u_strdup_printf ("%s/%s", uri_repository, namelist[n]->d_name);
			FILE *subs = fopen(subs_path, "r");
			u_free(subs_path);
			count = 0;
			buf = NULL;
			while(!feof(subs)) {
				memset(block, 0, 512);
				m = fread(block, 1, 511, subs);
				if(m > 0) {
					pre_count = count;
					count += m;
					buf = u_realloc(buf, count);
					memcpy(buf+pre_count, block, m);
				}
			}
			fclose(subs);
			SubsRepositoryEntryH entry = u_malloc(sizeof(*entry));
			if(entry) {
				entry->strdoc = buf;
				entry->len = count;
				entry->uuid = u_strdup(namelist[n]->d_name);
				node = lnode_create(entry);
            			list_append(subscription_list, node);
            			debug("subscription file found: %s", namelist[n]->d_name );
			}
            		u_free(namelist[n]);
        	}
       	 u_free(namelist);
    	}
	return 0;
}
int LocalSubscriptionOpSave (char * uri_repository, char * uuid, unsigned char *subscriptionDoc)
{
	char buf[U_NAME_MAX];
	if(LocalSubscriptionInitFlag == 0) return -1;
	snprintf(buf, U_NAME_MAX, "%s/uuid:%s", uri_repository, uuid);
	FILE *subsfile = fopen(buf, "w");
	if(subsfile == NULL) return -1;
	fprintf(subsfile, "%s", subscriptionDoc);
	fclose(subsfile);
	return 0;
}
int LocalSubscriptionOpUpdate(char * uri_repository, char * uuid, char *expire)
{
	char buf[U_NAME_MAX];
	if(LocalSubscriptionInitFlag == 0) return -1;
	snprintf(buf, U_NAME_MAX, "%s/uuid:%s", uri_repository, uuid);
	WsXmlDocH doc= NULL;
	char *temp;
	int len = 0;
	doc = xml_parser_file_to_doc( buf, "UTF-8", 0);
	if(doc) {
		WsXmlNodeH node = ws_xml_get_child(ws_xml_get_soap_body(doc),
			0, XML_NS_EVENTING, WSEVENT_SUBSCRIBE);
		node = ws_xml_get_child(node, 0, XML_NS_EVENTING, WSEVENT_EXPIRES);
		ws_xml_set_node_text(node, expire);
		ws_xml_dump_memory_enc(doc, &temp, &len, "UTF-8");
		FILE *subsfile = fopen(buf, "w");
		fprintf(subsfile, "%s", temp);
		fclose(subsfile);
		ws_xml_free_memory(temp);
		ws_xml_destroy_doc(doc);
	}
	return 0;
}
int LocalSubscriptionOpDelete (char * uri_repository, char * uuid)
{
	char buf[U_NAME_MAX];
	if(LocalSubscriptionInitFlag == 0) return -1;
	snprintf(buf, U_NAME_MAX, "%s/uuid:%s", uri_repository, uuid);
	int r = unlink(buf);
	if(r) {
		debug("unlink %s failed! %s", buf, strerror(errno));
	}
	return 0;
}





































