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
 * @author Anas Nashif
 */
#ifdef HAVE_CONFIG_H
#include "wsman_config.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifndef WIN32
#include <dlfcn.h>
#endif
#include "u/libu.h"
#include "wsman-faults.h"
#include "wsman-xml-api.h"
#include "wsman-plugins.h"
#include "wsman-server.h"
#include "wsman-soap.h"
#include "wsman-dispatcher.h"

extern struct __SubsRepositoryOpSet subscription_repository_op_set;

WsManListenerH *wsman_dispatch_list_new()
{
	WsManListenerH *list =
	    (WsManListenerH *) u_malloc(sizeof(WsManListenerH));
	return list;
}

#if 0
void wsman_server_read_plugin_config(void *arg, char *config_file)
{
	lnode_t *node;
	SoapH soap = (SoapH) arg;
	WsManListenerH * listener = ((WsManListenerH *)soap->listener;
	node = list_first(listener->plugins);
	while (node) {
		WsManPlugin *p = (WsManPlugin *) node->list_data;
		p->set_config = dlsym(p->p_handle, "set_config");
		if (listener->config && p->set_config) {
			p->set_config(p->p_handle, listener->config);
		} else {
			debug("no configuration available for plugin: %s", p->p_name);
		}
	}
}


#endif

static int wsman_server_verify_plugin(WsDispatchInterfaceInfo *ifcinfo)
{
	debug("Plugin '%s', version: %s", (ifcinfo->displayName), (ifcinfo->version) );
	if (strcmp (PACKAGE_VERSION, ifcinfo->version) == 0) {
		return 1;
	}
	return 0;
}



WsContextH wsman_init_plugins(WsManListenerH * listener)
{
	list_t *list = list_create(LISTCOUNT_T_MAX);
	lnode_t *node;
	WsContextH cntx = NULL;
	WsDispatchInterfaceInfo *ifcinfo = NULL;
	wsman_plugins_load(listener);
	node = list_first(listener->plugins);

	while (node) {
		WsManPlugin *p = (WsManPlugin *) node->list_data;
		p->ifc =
		    (WsDispatchInterfaceInfo *)
		    malloc(sizeof(WsDispatchInterfaceInfo));

		p->set_config = dlsym(p->p_handle, "set_config");
		p->get_endpoints = dlsym(p->p_handle, "get_endpoints");


		if (listener->config && p->set_config) {
			p->set_config(p->p_handle, listener->config);
		} else {
			debug("no configuration available for plugin: %s", p->p_name);
		}

		if (p->get_endpoints)
			p->get_endpoints(p->p_handle, p->ifc);

		ifcinfo = p->ifc;
		if (p->ifc && wsman_server_verify_plugin(ifcinfo)) {
			lnode_t *i = lnode_create(p->ifc);
			list_append(list, i);
		} else {
			error ("Plugin is not compatible with version of the software or plugin is invalid");
			error("invalid plugins");
		}
		node = list_next(listener->plugins, node);
	}
	cntx = ws_create_runtime(list);
	return cntx;
}


SubsRepositoryOpSetH wsman_init_subscription_repository(WsContextH cntx, char *uri)
{
	SoapH soap = ws_context_get_runtime(cntx);
	if(soap) {
		soap->subscriptionOpSet = &subscription_repository_op_set;
		if(uri) {
			soap->uri_subsRepository = u_strdup(uri);
			soap->subscriptionOpSet->init_subscription(uri, NULL);
		}
	}
	return &subscription_repository_op_set;
}


void *wsman_server_auxiliary_loop_thread(void *arg)
{
	WsContextH cntx = (WsContextH) arg;
	int r;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	struct timespec timespec;
	struct timeval tv;

	if ((r = pthread_cond_init(&cond, NULL)) != 0) {
		error("pthread_cond_init failed = %d", r);
		return NULL;
	}
	if ((r = pthread_mutex_init(&mutex, NULL)) != 0) {
		error("pthread_mutex_init failed = %d", r);
		return NULL;
	}

	while (continue_working) {
		pthread_mutex_lock(&mutex);
		gettimeofday(&tv, NULL);
		timespec.tv_sec = tv.tv_sec + 1;
		timespec.tv_nsec = tv.tv_usec * 1000;
		pthread_cond_timedwait(&cond, &mutex, &timespec);
		pthread_mutex_unlock(&mutex);

		wsman_timeouts_manager(cntx, NULL);
		wsman_heartbeat_generator(cntx, NULL);
	}
	return NULL;
}

