
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <wsman-client-transport.h>
#include <wsman-xml-api.h>
#include <wsman-client-api.h>

typedef struct {
	int			id;
	WsManClient		*client;
	actionOptions		options;
	char			*resource_uri;
	char			*enum_context;
	list_t			*eprs;
	pthread_mutex_t		mutex;
} session_t;

static list_t *sessions = NULL;
static pthread_mutex_t lock;

static void default_auth_callback(ws_auth_type_t auth, char **user, char **pwd)
{
	*user = strdup("");
}


static void _collect_epr(session_t *s, WsXmlDocH doc)
{
	WsXmlNodeH		node, resu, selector;
	char			*resource_uri = NULL, *name, *value, *it;
	int			index = 0;
	epr_t			*epr;
	selector_t		*sel;

	if (!doc || !s) {
		return;
	}

	node = ws_xml_get_soap_body(doc);

	if (node != NULL) {
		node = ws_xml_get_child(node, 0,
				XML_NS_ENUMERATION, WSENUM_PULL_RESP);
	} else {
		return;
	}

	if (node != NULL ) {
		node = ws_xml_get_child(node, 0,
				XML_NS_ENUMERATION, WSENUM_ITEMS);
	} else {
		return;
	}

	if (node != NULL) {
		it = ws_xml_get_node_text(node);
		node =  ws_xml_get_child(node, 0,  XML_NS_ADDRESSING, WSA_EPR);
	} else {
		return;
	}

	if (node != NULL) {
		node =  ws_xml_get_child(node, 0,
				XML_NS_ADDRESSING, WSA_REFERENCE_PARAMETERS);
	} else {
		return;
	}

	if (node != NULL) {
		resu = ws_xml_get_child(node, 0,
				XML_NS_WS_MAN, WSM_RESOURCE_URI);
		if (resu) {
			resource_uri = ws_xml_get_node_text(resu);
		}
		node = ws_xml_get_child(node, 0,
				XML_NS_WS_MAN, WSM_SELECTOR_SET);
		if (!node) {
			return;
		}
	} else { 
		return;
	}

	epr = u_malloc(sizeof(epr_t));
	epr->resource_uri = u_strdup(resource_uri);
	epr->selectors = list_create(LISTCOUNT_T_MAX);

	while ((selector = ws_xml_get_child(node, index++,
					XML_NS_WS_MAN, WSM_SELECTOR))) {
		name = ws_xml_find_attr_value(selector,
					XML_NS_WS_MAN, WSM_NAME);
		if (name == NULL) {
			name = ws_xml_find_attr_value(selector, NULL, WSM_NAME);
			if (name) {
				value = ws_xml_get_node_text(selector);
				sel = u_malloc(sizeof(selector_t));
				sel->key = u_strdup(name);
				sel->value = u_strdup(value);
				list_append(epr->selectors, lnode_create(sel));
			}
		}
	}

	list_append(s->eprs, lnode_create(epr));

	return;
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
			const char *password)
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

	s->eprs = list_create(LISTCOUNT_T_MAX);
	initialize_action_options(&s->options);

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

int wsman_session_uri_set(int session_id, const char *resource_uri)
{
	session_t	*s;

	if (!resource_uri) {
		return 0;
	}

	s = get_session_by_id(session_id);

	if (!s) {
		return 0;
	}

	pthread_mutex_lock(&s->mutex);
	if (s->resource_uri && resource_uri) {
		if (!strcasecmp(s->resource_uri, resource_uri)) {
			pthread_mutex_unlock(&s->mutex);
			return 1;
		}
	}

//	wsman_session_enum_context_free(session_id);
/* FIX ME */
	if (s->resource_uri)
		u_free(s->resource_uri);
	if (resource_uri) {
		s->resource_uri = u_strdup(resource_uri);
	} else {
		s->resource_uri = NULL;
	}

	pthread_mutex_unlock(&s->mutex);

	return 1;
}

char* wsman_session_resource_uri_get(int session_id)
{
	session_t	*s;

	s = get_session_by_id(session_id);

	if (!s) {
		return NULL;
	}

	if (s->resource_uri)
		return u_strdup(s->resource_uri);
	else
		return NULL;
}

list_t* wsman_session_get_eprs(int session_id)
{
	session_t	*s;

	s = get_session_by_id(session_id);

	if (!s) {
		return NULL;
	}

	return s->eprs;
}

static WsXmlDocH _request_create(session_t *s, WsmanAction action)
{
								/* FIX ME */
	return wsman_client_create_request(s->client, s->resource_uri,
				s->options, action, NULL, s->enum_context);

}

WsXmlDocH wsman_session_request_create(int session_id, WsmanAction action)
{
	session_t		*s;
	WsXmlDocH		req;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->mutex);
	req = _request_create(s, action);
	pthread_mutex_unlock(&s->mutex);

	return req;
}

static WsmanAction _get_action_from_doc(WsXmlDocH doc)
{
	char           *ptr = NULL,	*act;
	WsXmlNodeH	node;

	if (doc) {
		node = ws_xml_get_soap_header(doc);
		node = ws_xml_get_child(node, 0,
					XML_NS_ADDRESSING, WSA_ACTION);
		ptr = (!node) ? NULL : ws_xml_get_node_text(node);
	}

	if (ptr) {
		act = strrchr(ptr, '/') + 1;
		if (!strcmp(act, WSENUM_ENUMERATE)) {
			return WSMAN_ACTION_ENUMERATION;
		} else if (!strcmp(act, WSENUM_PULL)) {
			return WSMAN_ACTION_PULL;
		} else {
			return WSMAN_ACTION_NONE;
		}
		u_free(ptr);
	} else {
	}

	return WSMAN_ACTION_NONE;
}

static void _request_send(session_t *s, WsXmlDocH request, wsman_data_t	**dtp)
{
	wsman_data_t		*data = *dtp;
	req_resp_t		*reqresp;

	if (!data) {
		data = u_zalloc(sizeof(wsman_data_t));
		data->reqresp = list_create(LISTCOUNT_T_MAX);
		*dtp = data;
	}
	reqresp = u_zalloc(sizeof(req_resp_t));
	list_append(data->reqresp, lnode_create(reqresp));

	reqresp->request = request;

	wsman_send_request(s->client, request);

	reqresp->response = wsman_build_envelope_from_response(s->client);
	data->response_code = wsman_client_get_response_code(s->client);

	if (reqresp->response) {
		switch (_get_action_from_doc(request)) {
		case WSMAN_ACTION_ENUMERATION:
		case WSMAN_ACTION_PULL:
			_collect_epr(s, reqresp->response);
			s->enum_context =
				wsenum_get_enum_context(reqresp->response);
						/* FIX ME */
			break;
		default:
			break;
		}
	} else {
		data->fault_message = wsman_client_get_fault_string(s->client);
	}
}

wsman_data_t* wsman_session_request_send(int session_id, WsXmlDocH request)
{
	session_t		*s;
	wsman_data_t		*data = NULL;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->mutex);
	_request_send(s, request, &data);
	pthread_mutex_unlock(&s->mutex);

	return data;
}

static void _do_action(session_t *s, WsmanAction action, wsman_data_t **dtp)
{
	_request_send(s, _request_create(s, action), dtp);
}

wsman_data_t* wsman_session_do_action(int session_id, WsmanAction action)
{
	session_t		*s;
	wsman_data_t		*data = NULL;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->mutex);
	_do_action(s, action, &data);
	pthread_mutex_unlock(&s->mutex);

	return data;
}

wsman_data_t* wsman_session_pull_all(int session_id)
{
	session_t		*s;
	wsman_data_t		*data = NULL;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->mutex);

	wsman_set_action_option(&(s->options), FLAG_ENUMERATION_ENUM_EPR);
	list_destroy_nodes(s->eprs); /* FIX ME */

	_do_action(s, WSMAN_ACTION_ENUMERATION, &data);
	wsman_set_action_option(&(s->options), FLAG_ENUMERATION_ENUM_EPR);
					/* FIX ME */

	while (s->enum_context) {
		_do_action(s, WSMAN_ACTION_PULL, &data);
	}

	pthread_mutex_unlock(&s->mutex);

	return data;
}




/* ------------------------------------------------------------------------- */

void wsmanu_print_data(wsman_data_t *data, char **request, char **response)
{
	char		*req = NULL, *resp = NULL;
	int		size;
	lnode_t		*node;
	req_resp_t	*reqresp;

	if (!data || !(data->reqresp)) {
		return;
	}
	if (!request && !response) {
		return;
	}

	node = list_first(data->reqresp);

/* FIX ME */
		reqresp = (req_resp_t *)node->list_data;
		ws_xml_dump_memory_node_tree(
			ws_xml_get_doc_root(reqresp->request), &req, &size);

		if (reqresp->response) {
			ws_xml_dump_memory_node_tree(
				ws_xml_get_doc_root(reqresp->response),
				&resp, &size);
		}

	if (request) {
		*request = req;
	} else {
		if (req)
			u_free(req);
	}

	if (response) {
		*response = resp;
	} else {
		if (resp)
			u_free(resp);
	}
}


char* wsmanu_print_request(wsman_data_t *data)
{
	char		*buf;

	wsmanu_print_data(data, &buf, NULL);

	return buf;
}

char* wsmanu_print_response(wsman_data_t *data)
{
	char		*buf;

	wsmanu_print_data(data, NULL, &buf);

	return buf;
}
