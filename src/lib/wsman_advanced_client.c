/*******************************************************************************
 * Copyright (C) 2004-2007 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,cl
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
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGclE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Denis Sadykov
 */

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-dispatcher.h"
#include "wsman-soap-envelope.h"

#include "wsman-xml.h"
#include "wsman-xml-serialize.h"
#include "wsman-client-transport.h"
#include "wsman-faults.h"
#include "wsman-client.h"

typedef struct {
	int			id;
	WsManClient		*client;
	int			flags;
	char			*fault;
	pthread_mutex_t		lock;
	char			*resource_uri;
/* For session */
	hash_t			*selectors;
/* For enumerator */
	char			*enum_context;
} session_t;

static list_t *sessions = NULL;
static list_t *enumerators = NULL;
static pthread_mutex_t lock;

static void default_auth_callback(ws_auth_type_t auth, char **user, char **pwd)
{
	*user = strdup("");
}

static void _init(void)
{
	if (pthread_mutex_init(&lock, NULL)) {
		exit(1);
	}

	sessions = list_create(LISTCOUNT_T_MAX);
	enumerators = list_create(LISTCOUNT_T_MAX);
	wsman_client_transport_init(NULL);
	ws_client_transport_set_auth_request_func(default_auth_callback);
}

static int compare_id(const void *node_data, const void *key)
{
	int	key_id = *(int *)key;
	int	node_id = ((session_t *)node_data)->id;

	return key_id == node_id ? 0 : 1;
}

static int get_free_id(list_t *list)
{
	int	i;

	for (i = 0; i < LISTCOUNT_T_MAX; i++) {
		if (list_find(list, &i, compare_id) == NULL) {
			return i;
		}
	}

	return -1;
}

static char _set_server(session_t *s,
			const char *server,
			int port,
			const char *path,
			const char *scheme,
			const char *username,
			const char *password)
{
	WsManClient	*client = wsman_create_client(server, port, path,
						scheme, username, password);

	if (client) {
		if (s->client) {
			wsman_transport_close_transport(s->client);
			wsman_release_client(s->client);
		}
		s->client = client;
		return 1;
	} else {
		return 0;
	}
}

static session_t* _session_by_id(list_t *list, int id)
{
	lnode_t		*snode;
	session_t	*s = NULL;

	pthread_mutex_lock(&lock);
	snode = list_find(list, &id, compare_id);
	if (snode) {
		s = (session_t *)snode->list_data;
	}
	pthread_mutex_unlock(&lock);

	return s;
}

static session_t* get_session_by_id(int sid)
{
	return _session_by_id(sessions, sid);
}

static session_t* get_enumerator_by_id(int eid)
{
	return _session_by_id(enumerators, eid);
}

static session_t* session_open(list_t *list,
				const char *server,
				int port,
				const char *path,
				const char *scheme,
				const char *username,
				const char *password,
				int flags)
{
	session_t		*s;
	int			sid;

	pthread_mutex_lock(&lock);

	if ((sid = get_free_id(list)) < 0) {
		pthread_mutex_unlock(&lock);
		return NULL;
	}

	s = u_zalloc(sizeof(session_t));

	if (!_set_server(s, server, port, path, scheme, username, password)) {
		u_free(s);
		pthread_mutex_unlock(&lock);
		return NULL;
	}

	if (pthread_mutex_init(&s->lock, NULL)) {
		u_free(s);
		pthread_mutex_unlock(&lock);
		return NULL;
	}
	s->id = sid;
	list_append(list, lnode_create(s));

	pthread_mutex_unlock(&lock);

	return s;
}

int wsman_session_open(const char *server,
			int port,
			const char *path,
			const char *scheme,
			const char *user,
			const char *pwd,
			int flags)
{
	session_t	*s;

	if (!sessions && !enumerators) {
		_init();
	}

	s = session_open(sessions, server, port, path,
			scheme, user, pwd, flags);

	if (s) {
		s->selectors = hash_create(HASHCOUNT_T_MAX, 0, 0 );
		return s->id;
	} else {
		return -1;
	}
}

static session_t* _session_clone(list_t *list, session_t *s)
{
	WsManClient	*cl = s->client;

	return session_open(list, cl->data.hostName, cl->data.port,
				cl->data.path, cl->data.scheme,
				cl->data.user, cl->data.pwd, s->flags);
}

static char session_close(list_t *list, int id)
{
	session_t	*s;
	lnode_t		*snode;

	pthread_mutex_lock(&lock);

	snode = list_find(list, &id, compare_id);
	if (!snode) {
		pthread_mutex_unlock(&lock);
		return 0;
	}

	s = (session_t *)snode->list_data;
	pthread_mutex_lock(&s->lock);

	if (s->client) {
		wsman_transport_close_transport(s->client);
		wsman_release_client(s->client);
	}

	list_delete(list, snode);

	pthread_mutex_unlock(&s->lock);
	if (s->selectors) {
		hash_free(s->selectors);
	}
	if (s->enum_context) {
		u_free(s->enum_context);
	}
	u_free(s);
	lnode_destroy(snode);

	pthread_mutex_unlock(&lock);

	return 1;
}

char wsman_session_close(int session_id)
{
	return session_close(sessions, session_id);
}

char wsman_session_set_server(int session_id,
				const char *server,
				int port,
				const char *path,
				const char *scheme,
				const char *username,
				const char *password)
{
	session_t	*s;
	char		ret;

	s = get_session_by_id(session_id);

	if (!s) {
		return 0;
	}

	pthread_mutex_lock(&s->lock);
	ret = _set_server(s, server, port, path, scheme, username, password);
	pthread_mutex_unlock(&s->lock);

	return ret;
}

static const char* session_error(session_t *s)
{
	char	*fault;

	pthread_mutex_lock(&s->lock);

	fault = wsman_client_get_fault_string(s->client);
	if (!fault && s->fault) {
		fault = s->fault;
	}

	pthread_mutex_unlock(&s->lock);

	return (const char *)fault;
}

const char* wsman_session_error(int session_id)
{

	session_t	*s;

	s = get_session_by_id(session_id);

	if (!s) {
		return NULL;
	}

	return session_error(s);
}

const char* wsman_enumerator_error(int enumerator_id)
{
	session_t	*s;

	s = get_enumerator_by_id(enumerator_id);

	if (!s) {
		return NULL;
	}

	return session_error(s);
}

static char _resource_locator_add_selector(session_t *s,
					const char *name,
					const char *value)
{
	if (!hash_lookup(s->selectors, name)) {
		if (!hash_alloc_insert(s->selectors, (char *)name,
					(char *)value)) {
			return 0;
		}
	}

	return 1;
}

char wsman_session_resource_locator_set(int session_id,
					const char *epr)
{
	session_t	*s;
	char		*name, *value;
	WsXmlDocH	doc;
	WsXmlNodeH	node, resuri_node = NULL, selector;
	char		*my_epr;
	int		index = 0;

	if (!epr) {
		return 0;
	} else {
		my_epr = u_strdup(epr);
	}

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	pthread_mutex_lock(&s->lock);

	doc = wsman_client_read_memory(s->client,
					my_epr,
					strlen(my_epr),
					NULL, 0);
	if (!doc) {
		s->resource_uri = my_epr;
		pthread_mutex_unlock(&s->lock);
		return 1;
	}
	u_free(my_epr);

	node =  ws_xml_get_child(ws_xml_get_doc_root(doc), 0,
				XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);

	if (node != NULL) {
		resuri_node = ws_xml_get_child(node, 0,
				XML_NS_WS_MAN, WSM_RESOURCE_URI);
		if (!resuri_node) {
			pthread_mutex_unlock(&s->lock);
			return 0;
		}
		node = ws_xml_get_child(node, 0,
				XML_NS_WS_MAN, WSM_SELECTOR_SET);
	}

	if (s->resource_uri) {
		u_free(s->resource_uri);
	}
	s->resource_uri = ws_xml_get_node_text(resuri_node);

	hash_free_nodes(s->selectors);
	while ((selector = ws_xml_get_child(node, index++,
					XML_NS_WS_MAN, WSM_SELECTOR))) {
		name = ws_xml_find_attr_value(selector,
					XML_NS_WS_MAN, WSM_NAME);
		if (name == NULL) {
			name = ws_xml_find_attr_value(selector, NULL, WSM_NAME);
			if (name) {
				value = ws_xml_get_node_text(selector);
				_resource_locator_add_selector(s, name, value);
			}
		}
	}
	pthread_mutex_unlock(&s->lock);

	return 1;
}

int wsman_session_resource_locator_new(int session_id, const char *epr)
{
	session_t	*s, *ns;

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	ns = _session_clone(sessions, s);

	if (!ns) {
		return -1;
	}

	ns->selectors = hash_create(HASHCOUNT_T_MAX, 0, 0 );
	if (wsman_session_resource_locator_set(ns->id, epr)) {
		return ns->id;
	} else {
		wsman_session_close(ns->id);
		return -1;
	}
}

char wsman_session_resource_locator_add_selector(int session_id,
						const char *name,
						const char *value)
{
	session_t	*s;

	if (!name || !value) {
		return 0;
	}

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	pthread_mutex_lock(&s->lock);
	_resource_locator_add_selector(s, name, value);
	pthread_mutex_unlock(&s->lock);

	return 1;
}

char wsman_session_resource_locator_clear_selectors(int session_id)
{
	session_t	*s;

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	pthread_mutex_lock(&s->lock);
	hash_free_nodes(s->selectors);
	pthread_mutex_unlock(&s->lock);

	return 1;
}

static char* _subcode_from_doc(WsXmlDocH doc)
{
	WsManFault	fault;
	char		*subcode = NULL;

	wsman_client_get_fault_data(doc, &fault);

	if (fault.subcode && strlen(fault.subcode) > 1) {
		subcode = u_strdup(fault.subcode);
	}

	return subcode;
}

static char _check_response(session_t *s, WsXmlDocH response)
{
	if (wsman_client_check_for_fault(response)) {
		s->fault = _subcode_from_doc(response);
		ws_xml_destroy_doc(response);
		return 1;
	}

	return 0;
}

char* wsman_session_identify(int session_id, int flag)
{
	session_t	*s;
	WsXmlDocH	request, response;
	WsXmlNodeH	node;
	actionOptions	*options = NULL;
	char		*res = NULL;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);

	options = initialize_action_options();

	request = wsman_client_create_request(s->client,
						NULL,
						options,
						WSMAN_ACTION_IDENTIFY,
						NULL, NULL);

	if (!request) {
		pthread_mutex_unlock(&s->lock);
		return NULL;
	}

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (_check_response(s, response)) {
			pthread_mutex_unlock(&s->lock);
			return NULL;
		}
		node = ws_xml_get_soap_body(response);
		res = wsman_client_node_to_formatbuf(
				ws_xml_get_child(node, 0 , NULL, NULL ));
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return res;
}


int wsman_session_enumerate(int session_id,
			const char *resource_uri,
			const char *filter,
			const char *dialect,
			int flags)
{
	session_t	*s, *e = NULL;
	WsXmlDocH	request, response;
	actionOptions	*options = NULL;
	char		*enum_context;

	s = get_session_by_id(session_id);
	if (!s) {
		return -1;
	}

	pthread_mutex_lock(&s->lock);

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	options = initialize_action_options();

	if (filter)
		options->filter = u_strdup(filter);
	if (dialect)
		options->dialect = u_strdup(dialect);
	options->flags = flags;
	if (resource_uri) {
		request = wsman_client_create_request(s->client,
						resource_uri,
						options,
						WSMAN_ACTION_ENUMERATION,
						NULL, NULL);
	} else {
		request = wsman_client_create_request(s->client,
					s->resource_uri,
					options,
					WSMAN_ACTION_ENUMERATION,
					NULL, NULL);
	}

	if (options->filter)
		u_free(options->filter);
	if (options->dialect)
		u_free(options->dialect);

	if (!request) {
		pthread_mutex_unlock(&s->lock);
		return -1;
	}

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (_check_response(s, response)) {
			pthread_mutex_unlock(&s->lock);
			return -1;
		}
		enum_context = wsenum_get_enum_context(response);
		if (enum_context) {
			if (strlen(enum_context) > 1) {
				e = _session_clone(enumerators, s);
			}
			if (e) {
				e->resource_uri = u_strdup(resource_uri);
				e->enum_context = enum_context;
			} else {
				u_free(enum_context);
			}
		}
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return e->id;
}

char wsman_enumerator_release(int enumerator_id)
{
	return session_close(enumerators, enumerator_id);
}

static char* _item_from_doc(WsXmlDocH doc)
{
	WsXmlNodeH		node;
	char			*item = NULL;

	node = ws_xml_get_soap_body(doc);
	node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_PULL_RESP);
	node = ws_xml_get_child(node, 0, XML_NS_ENUMERATION, WSENUM_ITEMS);

	if (node == NULL) {
		return NULL;
	}

	item = wsman_client_node_to_formatbuf(
			ws_xml_get_child(node, 0 , NULL, NULL ));

	return item;
}

char* wsman_enumerator_pull(int enumerator_id)
{
	session_t	*s;
	WsXmlDocH	request, response;
	actionOptions	*options = NULL;
	char		*enum_context;
	char		*item = NULL;

	s = get_enumerator_by_id(enumerator_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	options = initialize_action_options();
	request = wsman_client_create_request(s->client,
					s->resource_uri,
					options, WSMAN_ACTION_PULL,
					NULL, s->enum_context);

	if (s->enum_context) {
		u_free(s->enum_context);
		s->enum_context = NULL;
	}

	if (!request) {
		pthread_mutex_unlock(&s->lock);
		return NULL;
	}	

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (_check_response(s, response)) {
			pthread_mutex_unlock(&s->lock);
			return NULL;
		}
		item = _item_from_doc(response);
		enum_context = wsenum_get_enum_context(response);
		if (enum_context) {
			if (strlen(enum_context) > 1) {
				s->enum_context = enum_context;
			} else {
				u_free(enum_context);
			}
		}
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return item;
}

char wsman_enumerator_end(int enumerator_id)
{
	session_t	*s;

	s = get_enumerator_by_id(enumerator_id);
	if (!s) {
		return 0;
	}

	if (s->enum_context) {
		return 1;
	} else {
		wsman_enumerator_release(enumerator_id);
	}

	return 0;
}

char* wsman_session_transfer_get(int session_id,
				int flags)
{
	session_t	*s;
	WsXmlDocH	request, response;
	WsXmlNodeH	node;
	actionOptions	*options = NULL;
	char		*resource = NULL;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);
	options = initialize_action_options();
	options->selectors = s->selectors;

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	request = wsman_client_create_request(s->client, s->resource_uri,
					options, WSMAN_ACTION_TRANSFER_GET,
					NULL, NULL);

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (_check_response(s, response)) {
			pthread_mutex_unlock(&s->lock);
			return NULL;
		}
		node = ws_xml_get_soap_body(response);
		resource = wsman_client_node_to_formatbuf(
				ws_xml_get_child(node, 0 , NULL, NULL ));
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return resource;
}

char* wsman_session_transfer_put(int session_id,
				const char *xml_content,
				int flags)
{
	session_t	*s;
	WsXmlDocH	request,	response;
	WsXmlDocH 	doc_content;
	WsXmlNodeH	node;
	actionOptions	*options = NULL;
	char		*resource = NULL;

	if (!xml_content) {
		return NULL;
	}

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);

	doc_content = wsman_client_read_memory(s->client,
					u_strdup(xml_content),
					strlen(xml_content),
					NULL, 0);
	if (!doc_content) {
		pthread_mutex_unlock(&s->lock);
		return NULL;
	};

	options = initialize_action_options();
	options->selectors = s->selectors;

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	request = wsman_client_create_request(s->client, s->resource_uri,
					options, WSMAN_ACTION_TRANSFER_PUT,
					NULL, NULL);
	if (!request) {
		ws_xml_destroy_doc(doc_content);
		pthread_mutex_unlock(&s->lock);
		return NULL;
	}

	ws_xml_copy_node(ws_xml_get_doc_root(doc_content),
			ws_xml_get_soap_body(request));

	wsman_send_request(s->client, request);

//	ws_xml_destroy_doc(doc_content);
//	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (_check_response(s, response)) {
			pthread_mutex_unlock(&s->lock);
			return NULL;
		}
		node = ws_xml_get_soap_body(response);
		resource = wsman_client_node_to_formatbuf(
				ws_xml_get_child(node, 0 , NULL, NULL ));
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return resource;
}

char* wsman_session_transfer_create(int session_id,
				const char *xml_content,
				int flag)
{
	session_t	*s;
	WsXmlDocH	request,	response;
	WsXmlDocH 	doc_content;
	WsXmlNodeH	node;
	actionOptions	*options = NULL;
	char		*resource = NULL;

	if (!xml_content) {
		return NULL;
	}

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);

	doc_content = wsman_client_read_memory(s->client,
					u_strdup(xml_content),
					strlen(xml_content),
					NULL, 0);
	if (!doc_content) {
		pthread_mutex_unlock(&s->lock);
		return NULL;
	};

	options = initialize_action_options();

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	request = wsman_client_create_request(s->client, s->resource_uri,
					options, WSMAN_ACTION_TRANSFER_CREATE,
					NULL, NULL);
	if (!request) {
		ws_xml_destroy_doc(doc_content);
		pthread_mutex_unlock(&s->lock);
		return NULL;
	}

	ws_xml_duplicate_tree(ws_xml_get_soap_body(request),
				ws_xml_get_doc_root(doc_content));

	wsman_send_request(s->client, request);

	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (_check_response(s, response)) {
			pthread_mutex_unlock(&s->lock);
			return NULL;
		}
		node = ws_xml_get_soap_body(response);
		resource = wsman_client_node_to_formatbuf(
				ws_xml_get_child(node, 0 , NULL, NULL ));
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return resource;
}

char* wsman_session_invoke(int sid,
			const char *method,
			const char *xml_content,
			int flag)
{
	session_t	*s;
	WsXmlDocH	request,	response;
	WsXmlDocH 	doc_content;
	actionOptions	*options = NULL;
	char		*res = NULL;

	s = get_session_by_id(sid);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);

	doc_content = wsman_client_read_memory(s->client,
					u_strdup(xml_content),
					strlen(xml_content),
					NULL, 0);
	if (!doc_content) {
		pthread_mutex_unlock(&s->lock);
		return NULL;
	};

	options = initialize_action_options();
	options->selectors = s->selectors;

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	request = wsman_client_create_request(s->client, s->resource_uri,
					options, WSMAN_ACTION_CUSTOM,
					(char *)method, NULL);
	if (!request) {
		pthread_mutex_unlock(&s->lock);
		ws_xml_destroy_doc(doc_content);
		return NULL;
	}

	ws_xml_duplicate_tree(ws_xml_get_child(ws_xml_get_soap_body(request),
						0, NULL, NULL),
				ws_xml_get_doc_root(doc_content));
	ws_xml_destroy_doc(doc_content);

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (_check_response(s, response)) {
			pthread_mutex_unlock(&s->lock);
			return NULL;
		}
		res = wsman_client_node_to_formatbuf(
				ws_xml_get_child(ws_xml_get_soap_body(response),
						0 , NULL, NULL));
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return res;
}


char* wsman_session_serialize(int sid, void *data, void *type_info)
{

	session_t	*s;
	char		*class = NULL;
	WsXmlDocH	doc;
	WsXmlNodeH	node;
	char		*result = NULL;

	s = get_session_by_id(sid);
	if (!s) {
		return NULL;
	}

	doc = wsman_create_doc((s->client)->wscntx, "doc");

	if (s->resource_uri) {
		class = u_strdup(strrchr(s->resource_uri, '/') + 1);
	} else {
		class = u_strdup("class");
	}

	ws_serialize((s->client)->wscntx, ws_xml_get_doc_root(doc),
				data, (XmlSerializerInfo *) type_info,
				class, s->resource_uri, NULL, 1);
	ws_serializer_free_mem((s->client)->wscntx, data,
				(XmlSerializerInfo *) type_info);
	if (class) {
		u_free(class);
	}

	node = ws_xml_get_child(ws_xml_get_doc_root(doc), 0, NULL, NULL);

	if (node) {
		result = wsman_client_node_to_formatbuf(node);
	}
	ws_xml_destroy_doc(doc);

	return result;
}
