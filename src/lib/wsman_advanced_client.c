#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include "wsman-xml-api.h"
#include "wsman-client-transport.h"
#include "wsman-client.h"
#include "wsman-client-api.h"

enum {
	WSMAN_SESSION = 0,
	WSMAN_ENUMERATOR,
	WSMAN_RESOURCE_LOCATOR
};

typedef struct {
	int			id;
	int			type;
	WsManClient		*client;
	int			flags;
	char			*resource_uri;
	hash_t			*selectors;
	char			*enum_context;
	char			*fault;
	pthread_mutex_t		lock;
} session_t;

static list_t *sessions = NULL;
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
	wsman_client_transport_init(NULL);
	ws_client_transport_set_auth_request_func(default_auth_callback);
}

/*
static void _fini(void)
{
	pthread_mutex_destroy(&lock);
}
*/

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
			/* FIX ME */
//		wsman_transport_close_transport(s->client);
//		wsman_release_client(s->client);
		s->client = client;
		return 1;
	} else {
		return 0;
	}
}

static session_t* get_session_by_id(int sid)
{
	lnode_t		*snode;
	session_t	*s = NULL;

	pthread_mutex_lock(&lock);
	snode = list_find(sessions, &sid, compare_id);
	if (snode) {
		s = (session_t *)snode->list_data;
	}
	pthread_mutex_unlock(&lock);

	return s;
}

static int session_open(const char *server,
			int port,
			const char *path,
			const char *scheme,
			const char *username,
			const char *password,
			int flags,
			int type,
			const char *resource_uri,
			char *enum_context)
{
	session_t		*s;
	int			sid;

	pthread_mutex_lock(&lock);

	if ((sid = get_free_id(sessions)) < 0) {
		pthread_mutex_unlock(&lock);
		return -1;
	}

	s = u_zalloc(sizeof(session_t));
	list_append(sessions, lnode_create(s));
	s->id = sid;

	pthread_mutex_init(&s->lock, NULL);

	_set_server(s, server, port, path, scheme, username, password);
	s->flags = flags;
	s->type = type;
	switch(type) {
	case WSMAN_RESOURCE_LOCATOR:
		s->resource_uri = u_strdup(resource_uri);
		s->selectors = hash_create(HASHCOUNT_T_MAX, 0, 0);
		break;
	case WSMAN_ENUMERATOR:
		s->resource_uri = u_strdup(resource_uri);
		s->enum_context = enum_context;
		break;
	default:
		break;
	}

	pthread_mutex_unlock(&lock);

	return sid;
}

int wsman_session_open(const char *server,
			int port,
			const char *path,
			const char *scheme,
			const char *user,
			const char *pwd,
			int flags)
{
	
	if (!sessions) {
		_init();	/* FIX ME */
	}

	return session_open(server, port, path, scheme, user, pwd, flags,
				WSMAN_SESSION, NULL, NULL);
}

static int _session_clone(session_t *s,
			int type,
			const char *resource_uri,
			char *enum_context)
{
	WsManClient		*cl = s->client;

	return session_open(cl->data.hostName, cl->data.port,
				cl->data.path, cl->data.scheme,
				cl->data.user, cl->data.pwd, s->flags,
				type, resource_uri, enum_context);
}

char wsman_session_close(int session_id)
{
	session_t	*s;
	lnode_t		*snode;

	pthread_mutex_lock(&lock);

	snode = list_find(sessions, &session_id, compare_id);
	if (!snode) {
		pthread_mutex_unlock(&lock);
		return 0;
	}

	s = (session_t *)snode->list_data;
	pthread_mutex_lock(&s->lock);

//	wsman_transport_close_transport(s->client);
//	wsman_release_client(s->client);

	list_delete(sessions, snode);

	pthread_mutex_unlock(&s->lock);
	u_free(s);	/* FIX ME */
	lnode_destroy(snode);
	pthread_mutex_unlock(&lock);

	return 1;
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

char* wsman_session_error(int session_id)
{

	session_t	*s;
	char		*fault;

	s = get_session_by_id(session_id);

	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);

	fault = wsman_client_get_fault_string(s->client);
	if (!fault && s->fault) {
		fault = u_strdup(s->fault);
	}

	pthread_mutex_unlock(&s->lock);

	return fault;
}

int wsman_session_resource_locator_create(int session_id,
					const char *resource_uri)
{
	session_t	*s;

	if (!resource_uri) {
		return -1;
	}

	s = get_session_by_id(session_id);
	if (!s) {
		return -1;
	}

	if (s->type != WSMAN_SESSION) {
		return -1;
	}

	return _session_clone(s, WSMAN_RESOURCE_LOCATOR, resource_uri, NULL);
}

char wsman_resource_locator_remove(int locator_id)
{
	return wsman_session_close(locator_id);
}

char wsman_resource_locator_add_selector(int locator_id,
					const char *name,
					const char *value)
{
	session_t	*s;

	s = get_session_by_id(locator_id);
	if (!s) {
		return 0;
	}
	if (s->type != WSMAN_RESOURCE_LOCATOR) {
		return 0;
	}

	pthread_mutex_lock(&s->lock);
	if (!hash_lookup(s->selectors, name)) {
		if (!hash_alloc_insert(s->selectors, (char *)name,
							(char *)value)) {
			pthread_mutex_unlock(&s->lock);
			return 0;
		}
	}
	pthread_mutex_unlock(&s->lock);

	return 1;
}

char wsman_resource_locator_clear_selectors(int locator_id)
{
	session_t	*s;

	s = get_session_by_id(locator_id);
	if (!s) {
		return 0;
	}
	if (s->type != WSMAN_RESOURCE_LOCATOR) {
		return 0;
	}

	pthread_mutex_lock(&s->lock);
	hash_free_nodes(s->selectors);		/* FIX ME */
	pthread_mutex_unlock(&s->lock);

	return 1;
}

char wsman_enumerator_release(int enumerator_id)
{
	return wsman_session_close(enumerator_id);
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


int wsman_session_enumerate(int session_id,
			const char *resource_uri,
			const char *filter,
			const char *dialect,
			int flags)
{
	session_t	*s;
	WsXmlDocH	request, response;
	actionOptions	options;
	char		*enum_context;
	int		eid = -1;

	s = get_session_by_id(session_id);
	if (!s) {
		return -1;
	}

	if (s->type != WSMAN_SESSION) {
		return -1;
	}

	pthread_mutex_lock(&s->lock);

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	initialize_action_options(&options);

	if (filter)
		options.filter = u_strdup(filter);
	if (dialect)
		options.dialect = u_strdup(dialect);
	options.flags = flags;
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

	if (options.filter)
		u_free(options.filter);
	if (options.dialect)
		u_free(options.dialect);

	if (!request) {
		pthread_mutex_unlock(&s->lock);
		return -1;
	}	

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (wsman_client_check_for_fault(response)) {
			s->fault = _subcode_from_doc(response);
			ws_xml_destroy_doc(response);
			pthread_mutex_unlock(&s->lock);
			return -1;
		}
		enum_context = wsenum_get_enum_context(response);
		if (enum_context) {
			eid = _session_clone(s, WSMAN_ENUMERATOR,
					resource_uri, enum_context);
		}
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return eid;
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
	actionOptions	options;
	char		*item = NULL;

	s = get_session_by_id(enumerator_id);
	if (!s) {
		return NULL;
	}
	if (s->type != WSMAN_ENUMERATOR) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	initialize_action_options(&options);
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
		if (wsman_client_check_for_fault(response)) {
			s->fault = _subcode_from_doc(response);
			ws_xml_destroy_doc(response);
			pthread_mutex_unlock(&s->lock);
			return NULL;
		}
		item = _item_from_doc(response);
		s->enum_context = wsenum_get_enum_context(response);
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->lock);

	return item;
}

char wsman_enumerator_end(int enumerator_id)
{
	session_t	*s;

	s = get_session_by_id(enumerator_id);
	if (!s) {
		return 0;
	}
	if (s->type != WSMAN_ENUMERATOR) {
		return 0;
	}

	if (s->enum_context) {
		return 1;
	} else {
		wsman_enumerator_release(enumerator_id);
	}

	return 0;
}

char* wsman_resource_locator_transfer_get(int locator_id,
					int flags)
{
	session_t	*s;
	WsXmlDocH	request, response;
	WsXmlNodeH	node;
	actionOptions	options;
	char		*resource = NULL;

	s = get_session_by_id(locator_id);
	if (!s) {
		return NULL;
	}
	if (s->type != WSMAN_RESOURCE_LOCATOR) {
		return NULL;
	}

	pthread_mutex_lock(&s->lock);
	initialize_action_options(&options);
	options.selectors = s->selectors;

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
		if (wsman_client_check_for_fault(response)) {
			s->fault = _subcode_from_doc(response);
			ws_xml_destroy_doc(response);
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

