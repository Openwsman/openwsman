#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <wsman-client-transport.h>
#include <wsman-xml-api.h>
#include <wsman-client-api.h>

typedef struct {
	char	*resource_uri;
	hash_t	*selectors;
	hash_t	*properties;
} res_locator_t;

typedef struct {
	char	*resource_uri;
	char	*enum_context;
} enumerator_t;

typedef struct {
	int			id;
	WsManClient		*client;
	res_locator_t		resource_locator;
	enumerator_t		enumerator;
	char			*fault;
	pthread_mutex_t		mutex;
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

static int get_free_id(void)
{
	int	i;

	for (i = 0; i < LISTCOUNT_T_MAX; i++) {
		if (list_find(sessions, &i, compare_id) == NULL) {
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

int wsman_session_open(const char *server,
			int port,
			const char *path,
			const char *scheme,
			const char *username,
			const char *password,
			int flags)
{
	session_t		*s;
	int			sid;

	if (!sessions) {
		_init();
	}

	pthread_mutex_lock(&lock);

	if ((sid = get_free_id()) < 0) {
		pthread_mutex_unlock(&lock);
		return -1;
	}

	s = u_zalloc(sizeof(session_t));
	list_append(sessions, lnode_create(s));
	
	pthread_mutex_init(&s->mutex, NULL);
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&s->mutex);

	s->id = sid;
	_set_server(s, server, port, path, scheme, username, password);

	s->resource_locator.selectors = hash_create(HASHCOUNT_T_MAX, 0, 0);
	s->resource_locator.properties = hash_create(HASHCOUNT_T_MAX, 0, 0);

	pthread_mutex_unlock(&s->mutex);

	return sid;
}

void wsman_session_close(int session_id)
{
	session_t	*s;
	lnode_t		*snode;

	pthread_mutex_lock(&lock);

	snode = list_find(sessions, &session_id, compare_id);
	if (!snode) {
		pthread_mutex_unlock(&lock);
		return;
	}

	s = (session_t *)snode->list_data;
	pthread_mutex_lock(&s->mutex);

//	wsman_transport_close_transport(s->client);
//	wsman_release_client(s->client);

	list_delete(sessions, snode);

	pthread_mutex_unlock(&s->mutex);
	u_free(s);	/* FIX ME */
	lnode_destroy(snode);
	pthread_mutex_unlock(&lock);
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

	pthread_mutex_lock(&s->mutex);
	ret = _set_server(s, server, port, path, scheme, username, password);
	pthread_mutex_unlock(&s->mutex);

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

	pthread_mutex_lock(&s->mutex);

	fault = wsman_client_get_fault_string(s->client);
	if (!fault && s->fault) {
		fault = u_strdup(s->fault);
	}

	pthread_mutex_unlock(&s->mutex);

	return fault;
}

const char* wsman_session_locator_resource_uri( int session_id,
						const char *resource_uri)
{
	session_t	*s;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	if (!resource_uri) {
		return (const char *)s->resource_locator.resource_uri;
	}

	pthread_mutex_lock(&s->mutex);

	if (s->resource_locator.resource_uri) {
		u_free(s->resource_locator.resource_uri);
	}
	s->resource_locator.resource_uri = u_strdup(resource_uri);

	pthread_mutex_unlock(&s->mutex);

	return (const char *)s->resource_locator.resource_uri;
}

char wsman_session_locator_add_selector(int session_id,
					const char *name,
					const char *value)
{
	session_t	*s;

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	if (!hash_lookup(s->resource_locator.selectors, name)) {
		if (!hash_alloc_insert(s->resource_locator.selectors,
					(char *)name, (char *)value)) {
			return 0;
		}
	} else {
		return 0;
	}

	return 1;
}

char wsman_session_locator_clear_selectors(int session_id)
{
	session_t	*s;

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	hash_free_nodes(s->resource_locator.selectors);

	return 1;
}

char wsman_session_locator_add_property(int session_id,
					const char *name,
					const char *value)
{
	session_t	*s;

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	if (!hash_lookup(s->resource_locator.properties, name)) {
		if (!hash_alloc_insert(s->resource_locator.properties,
					(char *)name, (char *)value)) {
			return 0;
		}
	} else {
		return 0;
	}

	return 1;
}

char wsman_session_locator_clear_properties(int session_id)
{
	session_t	*s;

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	hash_free_nodes(s->resource_locator.properties);

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


char* wsman_session_enumerate(  int session_id,
				const char *resource_uri,
				const char *filter,
				const char *dialect,
				int flags)
{
	session_t	*s;
	WsXmlDocH	request, response;
	actionOptions	options;
	char		*str = NULL;
	int		size;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->mutex);

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}
	if (s->enumerator.enum_context) {
		u_free(s->enumerator.enum_context);
		s->enumerator.enum_context = NULL;
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
					s->resource_locator.resource_uri,
					options,
					WSMAN_ACTION_ENUMERATION,
					NULL, NULL);
	}

	if (options.filter)
		u_free(options.filter);
	if (options.dialect)
		u_free(options.dialect);

	if (!request) {
		pthread_mutex_unlock(&s->mutex);
		return NULL;
	}	

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (wsman_client_check_for_fault(response)) {
			s->fault = _subcode_from_doc(response);
			ws_xml_destroy_doc(response);
			pthread_mutex_unlock(&s->mutex);
			return NULL;
		}
		ws_xml_dump_memory_node_tree(ws_xml_get_doc_root(response),
								&str, &size);
		s->enumerator.enum_context = wsenum_get_enum_context(response);
		if (s->enumerator.enum_context && resource_uri) {
			s->enumerator.resource_uri = u_strdup(resource_uri);
		}
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->mutex);

	return str;
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

char* wsman_session_enumerator_pull(int session_id)
{
	session_t	*s;
	WsXmlDocH	request, response;
	actionOptions	options;
	char		*item = NULL;

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	pthread_mutex_lock(&s->mutex);

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	initialize_action_options(&options);
	request = wsman_client_create_request(s->client,
					s->enumerator.resource_uri,
					options, WSMAN_ACTION_PULL,
					NULL, s->enumerator.enum_context);

	if (s->enumerator.enum_context) {
		u_free(s->enumerator.enum_context);
		s->enumerator.enum_context = NULL;
	}

	if (!request) {
		pthread_mutex_unlock(&s->mutex);
		return NULL;
	}	

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (wsman_client_check_for_fault(response)) {
			s->fault = _subcode_from_doc(response);
			ws_xml_destroy_doc(response);
			pthread_mutex_unlock(&s->mutex);
			return NULL;
		}
		item = _item_from_doc(response);
		s->enumerator.enum_context = wsenum_get_enum_context(response);
		if (!s->enumerator.enum_context &&
						s->enumerator.resource_uri) {
			u_free(s->enumerator.resource_uri);
		}
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->mutex);

	return item;
}

char wsman_session_enumerator_end(int session_id)
{
	session_t	*s;

	s = get_session_by_id(session_id);
	if (!s) {
		return 0;
	}

	if (s->enumerator.enum_context)
		return 1;

	return 0;
}

char* wsman_session_transfer_get(int session_id,
				const char *resource_uri,
				int flags)
{
	session_t	*s;
	WsXmlDocH	request, response;
	WsXmlNodeH	node;
	actionOptions	options;
	char		*resource = NULL;
	char		*uri = NULL, *sels = NULL;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	initialize_action_options(&options);

	if (resource_uri) {
		uri = u_strdup(resource_uri);
		uri = strtok_r(uri, "?", &sels);
		wsman_add_selectors_from_query_string(&options, sels);
	} else {
		if (s->resource_locator.resource_uri) {
			uri = u_strdup(s->resource_locator.resource_uri);
			options.selectors = s->resource_locator.selectors;
		}
	}

	pthread_mutex_lock(&s->mutex);

	if (s->fault) {
		u_free(s->fault);
		s->fault = NULL;
	}

	request = wsman_client_create_request(s->client, uri,
					options, WSMAN_ACTION_TRANSFER_GET,
					NULL, NULL);
	if (uri) {
		u_free(uri);
	}

	wsman_send_request(s->client, request);
	ws_xml_destroy_doc(request);

	response = wsman_build_envelope_from_response(s->client);

	if (response) {
		if (wsman_client_check_for_fault(response)) {
			s->fault = _subcode_from_doc(response);
			ws_xml_destroy_doc(response);
			pthread_mutex_unlock(&s->mutex);
			return NULL;
		}
		node = ws_xml_get_soap_body(response);
		resource = wsman_client_node_to_formatbuf(
				ws_xml_get_child(node, 0 , NULL, NULL ));
		ws_xml_destroy_doc(response);
	}

	pthread_mutex_unlock(&s->mutex);

	return resource;
}

