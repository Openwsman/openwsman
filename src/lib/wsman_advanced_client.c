
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

	return key_id == node_id ? 0 : -1;
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

static int _session_server_set(session_t *s,
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
	_session_server_set(s, server, port, path, scheme, username, password);

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

char* wsmcl_session_resource_uri_get(int session_id)
{
	session_t	*s;

	s = get_session_by_id(session_id);

	if (!s) {
		return NULL;
	}

	return u_strdup(s->resource_uri);
}

static wsman_data_t* _request_create(session_t *s, WsmanAction action)
{
	wsman_data_t		*data;

	data = u_zalloc(sizeof(wsman_data_t));

	data->request = wsman_client_create_request(s->client, action, NULL,
				s->resource_uri, s->options, s->enum_context);

	return data;
}

wsman_data_t* wsman_session_request_create(int session_id, WsmanAction action)
{
	session_t		*s;
	wsman_data_t		*data;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	pthread_mutex_lock(&s->mutex);
	data = _request_create(s, action);
	pthread_mutex_unlock(&s->mutex);

	return data;
}

wsman_data_t* wsman_session_request_send(int session_id, char *request)
{
	session_t		*s;
	wsman_data_t		*data;

	s = get_session_by_id(session_id);
	if (!s) {
		return NULL;
	}

	if (!request)
		return NULL;

	data = u_zalloc(sizeof(wsman_data_t));

	pthread_mutex_lock(&s->mutex);
	data->request = wsman_client_read_memory(s->client,
					request, strlen(request),
					NULL, 0);

	if(!data->request) {
		pthread_mutex_unlock(&s->mutex);
		u_free(data);
		return NULL;
	}

	wsman_send_request(s->client, data->request);

	data->response = wsman_build_envelope_from_response(s->client);
	data->response_code = wsman_get_client_response_code(s->client);

	if (data->response) {
		s->enum_context = wsenum_get_enum_context(data->response);
	} else {
		data->fault_message = wsman_client_get_fault_string(s->client);
	}

	pthread_mutex_unlock(&s->mutex);

	return data;
}

