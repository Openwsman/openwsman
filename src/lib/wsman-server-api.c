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
 * @author Vadim Revyakin
 */
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif
#ifndef WIN32
#include <dlfcn.h>
#endif

#include "u/libu.h"
#include "wsman-types.h"
#include "wsman-faults.h"
#include "wsman-soap-message.h"
#include "wsman-server-api.h"
#include "wsman-server.h"
#include "wsman-dispatcher.h"
#include "wsman-soap.h"
#include "wsman-plugins.h"
#ifdef ENABLE_EVENTING_SUPPORT
#include "wsman-cimindication-processor.h"
static char *uri_subsRepository;
#endif
#if 0
static void
debug_message_handler(const char *str,
		      debug_level_e level, void *user_data)
{

	int log_pid = getpid();

	char *log_name = u_strdup_printf("wsmand[%d]", log_pid);

	openlog(log_name, 0, LOG_DAEMON);
	syslog(LOG_INFO, "%s", str);
	closelog();
	u_free(log_name);
}
#endif

void wsman_server_read_plugin_config(void *arg, char *config_file)
{
	lnode_t *node;
	SoapH soap = (SoapH) arg;
	WsManListenerH * listener = (WsManListenerH *)soap->listener;
	if (config_file) {
		debug("reading config file 2");
		dictionary *ini;
		ini = iniparser_new(config_file);
		if (ini) {
			listener->config = ini;
		}
	}
	node = list_first(listener->plugins);
	while (node) {
		WsManPlugin *p = (WsManPlugin *) node->list_data;
		p->set_config = dlsym(p->p_handle, "set_config");
		if (listener->config && p->set_config) {
			p->set_config(p->p_handle, listener->config);
		} else {
			debug("no configuration available for plugin: %s", p->p_name);
		}
		node = list_next(listener->plugins, node);
	}
}

void *wsman_server_create_config(char *config_file)
{
	SoapH soap = NULL;
	dictionary *ini;
	WsManListenerH *listener = wsman_dispatch_list_new();
	WsContextH cntx;

	if (config_file) {
		debug("reading config file 1");
		ini = iniparser_new(config_file);
		if (ini) {
			listener->config = ini;
		}
	}
	cntx = wsman_init_plugins(listener);
	if (cntx != NULL) {
		soap = ws_context_get_runtime(cntx);
		if (listener)
			soap->listener = (WsManListenerH *)listener;
	}
	//debug_add_handler (debug_message_handler, DEBUG_LEVEL_ALWAYS, NULL);
	return (void *) soap;
}


void wsman_server_get_response(void *arg, void *msg)
{
	SoapH soap = (SoapH) arg;

	dispatch_inbound_call(soap,(WsmanMessage *)msg, NULL);
}
#ifdef ENABLE_EVENTING_SUPPORT
void wsman_server_set_subscription_repos(char *repos)
{
	uri_subsRepository = u_strdup(repos);
}

void *wsman_server_get_subscription_repos()
{
	return uri_subsRepository;
}

void wsman_event_init(void *arg)
{
	SoapH soap = (SoapH)arg;
	WsContextH cntx = soap->cntx;
	SubsRepositoryOpSetH ops = wsman_init_subscription_repository(cntx, (char *)wsman_server_get_subscription_repos());
	list_t *subs_list = list_create(-1);
	debug("subscription_repository_uri = %s", soap->uri_subsRepository);
	if(ops->load_subscription(soap->uri_subsRepository, subs_list) == 0) {
		lnode_t *node = list_first(subs_list);
		while(node) {
			SubsRepositoryEntryH entry = (SubsRepositoryEntryH)node->list_data;
			if(wsman_clean_subsrepository(cntx->soap, entry) == 0) {
				debug("load subscription %s", entry->uuid);
				wsman_repos_notification_dispatcher(cntx, entry, list_count(cntx->subscriptionMemList));
			}
			else
				u_free(entry->strdoc);
			u_free(entry->uuid);
			u_free(entry);
			list_delete(subs_list, node);
			lnode_destroy(node);
			node = list_first(subs_list);
		}
	}
	list_destroy(subs_list);
	wsman_init_event_pool(cntx, NULL);
}

void wsman_receive_cim_indication(void *arg, char *uuid, void *msg)
{
	SoapH soap = (SoapH) arg;
	CimxmlMessage *cimxml_msg = msg;
	cimxml_context *cntx = NULL;
	cntx = u_malloc(sizeof(cimxml_context));
	cntx->soap = soap;
	cntx->uuid = uuid;
	CIM_Indication_call(cntx, cimxml_msg, NULL);
}

#endif
