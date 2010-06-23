/*******************************************************************************
 * Copyright (C) 2004-2007 Intel Corp. All rights reserved.
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
#include "wsman_config.h"
#endif
#include "u/libu.h"
#include "wsman-event-pool.h"


int MemEventPoolInit (void *opaqueData);
int MemEventPoolFinalize (void *opaqueData);
int MemEventPoolCount(char *uuid);
int MemEventPoolAddEvent (char *uuid, WsNotificationInfoH notification);
int MemEventPoolAddPullEvent (char *uuid, WsNotificationInfoH notification) ;
int MemEventPoolGetAndDeleteEvent (char *uuid, WsNotificationInfoH *notification);
int MemEventPoolClearEvent (char *uuid, clearproc proc);

list_t *global_event_list = NULL;
int max_pull_event_number = 16;

struct __EventPoolOpSet event_pool_op_set ={MemEventPoolInit, MemEventPoolFinalize, 
	MemEventPoolCount, MemEventPoolAddEvent, MemEventPoolAddPullEvent,
	MemEventPoolGetAndDeleteEvent, MemEventPoolClearEvent};

EventPoolOpSetH wsman_get_eventpool_opset()
{
	return &event_pool_op_set;
}

int MemEventPoolInit (void *opaqueData) {
	global_event_list = list_create(-1);
	if(opaqueData)
		max_pull_event_number = *(int *)opaqueData;
	return 0;
}

int MemEventPoolFinalize (void *opaqueData)  {
	return 0;
}

int MemEventPoolCount(char *uuid) {
	lnode_t *node = NULL;
	event_entryH entry = NULL;
	node = list_first(global_event_list);
	while(node) {
		entry = (event_entryH)node->list_data;
		if(!strcasecmp(entry->subscription_id, uuid))
			break;
		node = list_next(global_event_list, node);
	}
	if(node)
		return list_count(entry->event_content_list);
	return 0;
}

int MemEventPoolAddEvent (char *uuid, WsNotificationInfoH notification) {
	lnode_t *node = NULL;
	event_entryH entry = NULL;
	if(notification == NULL) return 0;
	node = list_first(global_event_list);
	while(node) {
		entry = (event_entryH)node->list_data;
		if(!strcasecmp(entry->subscription_id, uuid))
			break;
		node = list_next(global_event_list, node);
	}
	if(node == NULL) { //No event_entry for this subscription, create it
		entry = u_malloc(sizeof(*entry));
		entry->event_content_list = list_create(-1);
		strcpy(entry->subscription_id, uuid);
		node = lnode_create(entry);
		list_append(global_event_list, node);
	}
	node = lnode_create(notification);
	list_append(entry->event_content_list, node);
	return 0;
}

int MemEventPoolAddPullEvent (char *uuid, WsNotificationInfoH notification) {
	lnode_t *node = NULL;
	event_entryH entry = NULL;
	if(notification == NULL) return 0;
	node = list_first(global_event_list);
	while(node) {
		entry = (event_entryH)node->list_data;
		if(!strcasecmp(entry->subscription_id, uuid))
			break;
		node = list_next(global_event_list, node);
	}
	if(node == NULL) { //No event_entry for this subscription, create it
		entry = u_malloc(sizeof(*entry));
		entry->event_content_list = list_create(-1);
		strcpy(entry->subscription_id, uuid);
		node = lnode_create(entry);
		list_append(global_event_list, node);
	}
	if(list_count(entry->event_content_list) > max_pull_event_number) 
		return -1;
	node = lnode_create(notification);
	list_append(entry->event_content_list, node);
	return 0;
}

int MemEventPoolGetAndDeleteEvent (char *uuid, WsNotificationInfoH *notification) {
	lnode_t *node = NULL;
	event_entryH entry = NULL;
	*notification = NULL;
	node = list_first(global_event_list);
	while(node) {
		entry = (event_entryH)node->list_data;
		if(!strcasecmp(entry->subscription_id, uuid))
			break;
		node = list_next(global_event_list, node);
	}
	if(node == NULL) { 
		return -1;
	}
	node = list_first(entry->event_content_list);
	if(node == NULL) return -1;
	list_delete(entry->event_content_list, node);
	*notification = (WsNotificationInfoH)node->list_data;
	lnode_destroy(node);
	return 0;
}

int MemEventPoolClearEvent (char *uuid, clearproc proc) {
	lnode_t *node = NULL;
	lnode_t *tmp = NULL;
	event_entryH entry = NULL;
	WsNotificationInfoH notification;
	node = list_first(global_event_list);
	while(node) {
		entry = (event_entryH)node->list_data;
		if(!strcasecmp(entry->subscription_id, uuid))
			break;
		node = list_next(global_event_list, node);
	}
	if(node == NULL) { 
		return -1;
	}
	list_delete(global_event_list, node);
	lnode_destroy(node);
	node = list_first(entry->event_content_list);
	while(node) {
		notification = (WsNotificationInfoH)node->list_data;
		if(proc)
			proc(notification);
		tmp = list_next(entry->event_content_list, node);
		list_delete(entry->event_content_list, node);
		lnode_destroy(node);
		node = tmp;
	}
	list_destroy(entry->event_content_list);
	u_free(entry);
	return 0;
}


