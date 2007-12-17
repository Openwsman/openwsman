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
 * @author Vadim Revyakin
 * @author Liang Hou
 */

#define _GNU_SOURCE
#ifdef HAVE_CONFIG_H
#include "wsman_config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef WIN32
#include <dlfcn.h>
#endif

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-soap-envelope.h"

#include "wsman-xml.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"


#include "shttpd.h"
#include "adapter.h"

#include "wsman-plugins.h"
#include "wsmand-listener.h"
#include "wsmand-daemon.h"
#include "wsman-server.h"
#include "wsman-server-api.h"
#include "wsman-plugins.h"
#ifdef ENABLE_EVENTING_SUPPORT
//#include "wsman-event-pool.h"
#include "wsman-cimindication-processor.h"
#endif

#define MULTITHREADED_SERVER

#ifdef MULTITHREADED_SERVER
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <sys/socket.h>

static pthread_mutex_t shttpd_mutex;
static pthread_cond_t shttpd_cond;
static list_t *request_list;
static int num_threads = 0;
static int min_threads = 2;
static int idle_threads = 0;
static int max_threads = 4;

#endif

int continue_working = 1;
static int (*basic_callback) (char *, char *) = NULL;


typedef struct {
	char *response;
	int length;
	int ind;
} ShttpMessage;


/* Check HTTP headers */
static
int check_request_content_type(struct shttpd_arg *arg) {
	const char *content_type;
	int status = WSMAN_STATUS_OK;

	content_type = shttpd_get_header(arg, "Content-Type");
	if (content_type && strncmp(content_type,
				    SOAP_CONTENT_TYPE,
				    strlen(SOAP_CONTENT_TYPE)) != 0) {
		status = WSMAN_STATUS_UNSUPPORTED_MEDIA_TYPE;
	}
	return status;
}

static
char *get_request_encoding(struct shttpd_arg *arg) {
	const char *content_type;
	char *p;
	char *encoding = NULL;

	content_type = shttpd_get_header(arg, "Content-Type");
	if(content_type ) {
		if(( p = strstr(content_type, "charset")) != NULL ) {
			p += strlen("charset");
			p++;
			encoding = p;
		}
	}

	return encoding;
}

static
void server_callback(struct shttpd_arg *arg)
{
	char *encoding = NULL;
	const char  *s;
	SoapH soap;
	int status = WSMAN_STATUS_OK;
	char *fault_reason = NULL;
    struct state {
        size_t  cl;     /* Content-Length   */
        size_t  nread;      /* Number of bytes read */
	 	u_buf_t     *request;
    } *state;

    /* If the connection was broken prematurely, cleanup */
    if ( (arg->flags & SHTTPD_CONNECTION_ERROR ) && arg->state) {
        free(arg->state);
		return;
    } else if ((s = shttpd_get_header(arg, "Content-Length")) == NULL) {
        shttpd_printf(arg, "HTTP/1.0 411 Length Required\n\n");
        arg->flags |= SHTTPD_END_OF_OUTPUT;
		return;
    } else if (arg->state == NULL) {
        /* New request. Allocate a state structure */
        arg->state = state = calloc(1, sizeof(*state));
        state->cl = strtoul(s, NULL, 10);
		u_buf_create(&(state->request));
    }

	state = arg->state;
	if (state->nread>0 )
		u_buf_append(state->request, arg->in.buf, arg->in.len);
	else
		u_buf_set(state->request, arg->in.buf, arg->in.len);

	state->nread += arg->in.len;
	arg->in.num_bytes = arg->in.len;
	if (state->nread >= state->cl) {
		arg->flags |= SHTTPD_END_OF_OUTPUT;
	} else {
		return;
	}


	/* Here we must handle the initial request */
	WsmanMessage *wsman_msg = wsman_soap_message_new();
	if ( (status = check_request_content_type(arg) ) != WSMAN_STATUS_OK ) {
		goto DONE;
	}
	if ( (encoding =  get_request_encoding(arg)) != NULL ) {
		wsman_msg->charset = u_strdup(encoding);
	}

	soap = (SoapH) arg->user_data;
	wsman_msg->status.fault_code = WSMAN_RC_OK;
	u_buf_set(wsman_msg->request, u_buf_ptr(state->request), u_buf_len(state->request));

	/*
	 * some plugins can use credentials for their own authentication
	 * works only with basic authentication
	 */
	 shttpd_get_credentials(arg, &wsman_msg->auth_data.username,
			       &wsman_msg->auth_data.password);

	/* Call dispatcher. Real request handling */
	if (status == WSMAN_STATUS_OK) {
		/* dispatch if we didn't find out any error */
		dispatch_inbound_call(soap, wsman_msg, NULL);
		status = wsman_msg->http_code;
	}

DONE:

	if (fault_reason == NULL) {
		fault_reason = shttpd_reason_phrase(status);
	}
	debug("Response status=%d (%s)", status, fault_reason);

	/*
	 * Here we begin to create the http response.
	 * Create the headers at first.
	 */

	shttpd_printf(arg, "HTTP/1.1 %d %s\r\n", status, fault_reason);
	shttpd_printf(arg, "Content-Type: application/soap+xml;charset=%s\r\n", encoding);
	shttpd_printf(arg, "Server: %s/%s\r\n", PACKAGE, VERSION);
	shttpd_printf(arg, "Content-Length: %d\r\n", u_buf_len(wsman_msg->response));
	shttpd_printf(arg, "\r\n");

	/* add response body to output buffer */

	if (wsman_msg->response) {
		shttpd_printf(arg, (char *)u_buf_ptr(wsman_msg->response) );
		shttpd_printf(arg, "\r\n\r\n");
	}

	wsman_soap_message_destroy(wsman_msg);
	debug("-> %s", arg->out.buf);

	u_buf_free(state->request);
	u_free(state);
	arg->flags |= SHTTPD_END_OF_OUTPUT;
	return;
}

#ifdef ENABLE_EVENTING_SUPPORT

static int build_cimxml_request(struct shttpd_arg *arg, CimxmlMessage *msg) {
	/* Get request from http server */
	char *body;
	size_t length;
	int status = WSMAN_STATUS_OK;

	body = u_strdup(arg->in.buf + arg->in.num_bytes);
	length = arg->in.len - arg->in.num_bytes;
	if (body == NULL) {
		status = WSMAN_STATUS_BAD_REQUEST;
		error("No request body. len = %d", length);
	} else {
		u_buf_construct(msg->request, body, length, length);
	}
	return status;
}

static void cimxml_listener_callback(struct shttpd_arg *arg)
{
	int status = CIMXML_STATUS_OK;
	int cim_error_code = 0;
	char *cim_error = NULL;
	char *fault_reason = NULL;
	char *uuid = NULL, *tmp, *end;
	cimxml_context *cntx = NULL;
	SoapH soap = NULL;
	char *encoding = NULL;
	if (arg->flags & SHTTPD_MORE_POST_DATA)
			return;
	CimxmlMessage *cimxml_msg = cimxml_message_new();
	tmp = (char *)shttpd_get_env(arg, "REQUEST_URI");
	if (tmp && ( end = strrchr(tmp, '/')) != NULL ) {
		uuid = &end[1];
	}
	if ( (encoding =  get_request_encoding(arg)) != NULL ) {
		cimxml_msg->charset = u_strdup(encoding);
	}
	const char *cimexport = shttpd_get_header(arg, "CIMExport");
	const char *cimexportmethod = shttpd_get_header(arg, "CIMExportMethod");
	if ( cimexportmethod && cimexport ) {
		if(strncmp(cimexport, "MethodRequest", strlen("MethodRequest")) ||
						strncmp(cimexportmethod, "ExportIndication", strlen("ExportIndication"))) {
		}
	} else {
		status = WSMAN_STATUS_FORBIDDEN;
		cim_error_code = CIMXML_STATUS_UNSUPPORTED_OPERATION;
		cim_error = "unsupported-operation";
		goto DONE;
	}
	soap = (SoapH) arg->user_data;
	if ((status = build_cimxml_request(arg, cimxml_msg ) ) != WSMAN_STATUS_OK ) {
			cim_error = "request-not-well-formed";
			status = WSMAN_STATUS_BAD_REQUEST;
			cim_error_code = CIMXML_STATUS_REQUEST_NOT_WELL_FORMED;
			goto DONE;
	}
	if (status == WSMAN_STATUS_OK) {
		cntx = u_malloc(sizeof(cimxml_context));
		cntx->soap = soap;
		cntx->uuid = uuid;
		CIM_Indication_call(cntx, cimxml_msg, NULL);
		status = cimxml_msg->http_code;
		cim_error_code = cimxml_msg->status.code;
		cim_error = cimxml_msg->status.fault_msg;
	}

DONE:
	if (fault_reason == NULL) {
		fault_reason = shttpd_reason_phrase(status);
	}
	debug("Response (status) %d (%s)", status, fault_reason);

	/*
	 * Here we begin to create the http response.
	 * Create the headers at first.
	 * We consider output buffer of server is large enough to hold all headers.
	 */
	shttpd_printf(arg, "HTTP/1.1 %d %s\r\n", status, fault_reason);
	shttpd_printf(arg, "Server: %s/%s\r\n", PACKAGE, VERSION);

	if (status != WSMAN_STATUS_OK) {
		if (cim_error) {
			shttpd_printf(arg, "CIMError: %s\r\n", cim_error);
		}
		shttpd_printf(arg, "\r\n%d %s\r\n", status, fault_reason);
		arg->flags |= SHTTPD_END_OF_OUTPUT;
		cimxml_message_destroy(cimxml_msg);
		debug("----> %s", arg->out.buf);
		return;
	}
	if ( u_buf_len(cimxml_msg->response) == 0) {
		/* can't send the body of response or nothing to send */
		shttpd_printf(arg, "\r\n");
		arg->flags |= SHTTPD_END_OF_OUTPUT;
		cimxml_message_destroy(cimxml_msg);
		return;
	}
	shttpd_printf(arg, "Content-Type: application/xml; charset=\"utf-8\"\r\n");
	shttpd_printf(arg, "Content-Length: %d\r\n", u_buf_len(cimxml_msg->response));
	shttpd_printf(arg, "CIMExport: MethodResponse\r\n");
	shttpd_printf(arg, "\r\n");
	shttpd_printf(arg,  (char *)u_buf_ptr(cimxml_msg->response) );
	shttpd_printf(arg, "\r\n\r\n");
	debug("-> %s", arg->out.buf);

	cimxml_message_destroy(cimxml_msg);
	arg->flags |= SHTTPD_END_OF_OUTPUT;
	return;
}


#endif


static void listener_shutdown_handler(void *p)
{
	int *a = (int *) p;
	debug("listener_shutdown_handler started");
	*a = 0;
}

static void protect_uri(struct shttpd_ctx *ctx, char *uri)
{
	if (wsmand_options_get_digest_password_file()) {
			shttpd_protect_uri(ctx, uri,
                   wsmand_options_get_digest_password_file(),NULL, 1);
		debug("Using Digest Authorization for %s:", uri);
	}
	if (basic_callback) {
		shttpd_protect_uri(ctx, uri, NULL,
						basic_callback, 0);
		debug("Using Basic Authorization %s for %s:",
		      wsmand_option_get_basic_authenticator()?
		      wsmand_option_get_basic_authenticator() :
		      wsmand_default_basic_authenticator(), uri);
	}
}

static struct shttpd_ctx *create_shttpd_context(SoapH soap)
{
	struct shttpd_ctx *ctx;
	if (wsmand_options_get_use_ssl()) {
		message("Using SSL");
		ctx = shttpd_init(NULL,
				  "ssl_certificate",
				  wsmand_options_get_ssl_cert_file(),
				  "auth_realm",
				  AUTHENTICATION_REALM,
				  NULL);
	} else {
		ctx = shttpd_init(NULL,
				  "auth_realm", AUTHENTICATION_REALM,
				   NULL);
	}
	if (ctx == NULL) {
		return NULL;
	}
	shttpd_register_uri(ctx, wsmand_options_get_service_path(),
			    server_callback, (void *) soap);
#if 0
	shttpd_register_uri(ctx, ANON_IDENTIFY_PATH,
			    server_callback, (void *) soap);
#endif


#ifdef ENABLE_EVENTING_SUPPORT
	message("Registered CIM Indication Listener: %s", DEFAULT_CIMINDICATION_PATH "/*");
	shttpd_register_uri(ctx, DEFAULT_CIMINDICATION_PATH "/*", cimxml_listener_callback,(void *)soap);
	protect_uri( ctx, DEFAULT_CIMINDICATION_PATH );
#endif

	protect_uri( ctx, wsmand_options_get_service_path());

	return ctx;
}

#ifdef MULTITHREADED_SERVER

static void handle_socket(int sock, SoapH soap)
{
	struct shttpd_ctx *ctx;

	debug("Thread %d handles sock %d", pthread_self(), sock);

	ctx = create_shttpd_context(soap);
	if (ctx == NULL) {
		(void) shutdown(sock, 2);
		close(sock);
		return;
	}
	if (wsmand_options_get_use_ssl()) {
		shttpd_add_socket(ctx, sock, 1);
	} else {
		shttpd_add_socket(ctx, sock, 0);
	}
	while (shttpd_active(ctx) && continue_working) {
		shttpd_poll(ctx, 100);
	}
	shttpd_fini(ctx);
	debug("Thread %d processed sock %d", pthread_self(), sock);
}


static void *service_connection(void *arg)
{
	lnode_t *node;
	int sock;
	SoapH soap = (SoapH) arg;

	debug("shttpd thread %d started. num_threads = %d",
	      pthread_self(), num_threads);
	pthread_mutex_lock(&shttpd_mutex);
	while (continue_working) {
		if (list_isempty(request_list)) {
			// no sockets to serve
			if (num_threads > min_threads) {
				debug("we have too many threads %d > %d"
				      " Thread %d is being completed",
				      num_threads, min_threads,
				      pthread_self());
				break;
			} else {
				idle_threads++;
				debug("Thread %d goes to idle state",
				      pthread_self());
				(void) pthread_cond_wait(&shttpd_cond,
							 &shttpd_mutex);
				idle_threads--;
				continue;
			}
		}
		node = list_del_first(request_list);
		sock = (int) ((char *) lnode_get(node) - (char *) NULL);
		pthread_mutex_unlock(&shttpd_mutex);
		lnode_destroy(node);
		handle_socket(sock, soap);
		pthread_mutex_lock(&shttpd_mutex);
	}
	num_threads--;
	debug("shttpd thread %d completed. num_threads = %d",
	      pthread_self(), num_threads);
	if (num_threads == 0 && continue_working == 0) {
		debug("last thread completed");
		pthread_cond_broadcast(&shttpd_cond);
	}
	pthread_mutex_unlock(&shttpd_mutex);
	return NULL;
}

#endif


static int initialize_basic_authenticator(void)
{
	char *auth;
	char *arg;
	void *hnd;
	int (*init) (char *);
	char *name;
	int should_return = 0;
	int res = 0;

	if (wsmand_options_get_basic_password_file() != NULL) {
		if ((wsmand_option_get_basic_authenticator() &&
		     (strcmp(wsmand_default_basic_authenticator(),
			     wsmand_option_get_basic_authenticator()))) ||
		    wsmand_option_get_basic_authenticator_arg()) {
			fprintf(stderr,
				"basic authentication is ambigious in config file\n");
			return 1;
		}
		auth = wsmand_default_basic_authenticator();
		arg = wsmand_options_get_basic_password_file();
	} else {
		auth = wsmand_option_get_basic_authenticator();
		arg = wsmand_option_get_basic_authenticator_arg();
	}

	if (auth == NULL) {
		// No basic authenticationame
		return 0;
	}

	if (auth[0] == '/') {
		name = auth;
	} else {
		name = u_strdup_printf("%s/%s", PACKAGE_AUTH_DIR, auth);
		should_return = 1;
	}

	hnd = dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
	if (hnd == NULL) {
		fprintf(stderr, "Could not dlopen %s\n", name);
		res = 1;
		goto DONE;
	}
	basic_callback = dlsym(hnd, "authorize");
	if (basic_callback == NULL) {
		fprintf(stderr, "Could not resolve authorize() in %s\n",
			name);
		res = 1;
		goto DONE;
	}

	init = dlsym(hnd, "initialize");
	if (init != NULL) {
		res = init(arg);
	}
      DONE:
	if (should_return) {
		u_free(name);
	}
	return res;
}


WsManListenerH *wsmand_start_server(dictionary * ini)
{
	WsManListenerH *listener = wsman_dispatch_list_new();
	listener->config = ini;
	WsContextH cntx = wsman_init_plugins(listener);
#ifdef ENABLE_EVENTING_SUPPORT
	wsman_event_init(cntx->soap);
#endif
#ifdef MULTITHREADED_SERVER
	int r;
	int sock;
	lnode_t *node;
	pthread_t thr_id;
#ifdef ENABLE_EVENTING_SUPPORT
	pthread_t notificationManager_id;
#endif
	pthread_attr_t pattrs;
	struct timespec timespec;
#else
	struct shttpd_ctx *ctx;

#endif

	if (cntx == NULL) {
		return listener;
	}
#ifndef HAVE_SSL
	if (wsmand_options_get_use_ssl()) {
		error("Server configured without SSL support");
		return listener;
	}
#endif
	SoapH soap = ws_context_get_runtime(cntx);
	ws_set_context_enumIdleTimeout(cntx,wsmand_options_get_enumIdleTimeout());
	if (initialize_basic_authenticator()) {
		return listener;
	}

	int lsn;
	int port;


	if (wsmand_options_get_use_ssl()) {
		message("Using SSL");
		if (wsmand_options_get_ssl_cert_file() &&
		    wsmand_options_get_ssl_key_file() &&
		    (wsmand_options_get_server_ssl_port() > 0)) {
			port = wsmand_options_get_server_ssl_port();
		} else {
			error("Not enough data to use SSL port");
			return listener;
		}
	} else {
		port = wsmand_options_get_server_port();
	}

	message("     Working on port %d", port);
	if (wsmand_options_get_digest_password_file()) {
		message("Using Digest Authorization");
	}
	if (basic_callback) {
		message("Using Basic Authorization %s",
			wsmand_option_get_basic_authenticator()?
			wsmand_option_get_basic_authenticator() :
			wsmand_default_basic_authenticator());
	}

	if ((wsmand_options_get_digest_password_file() == NULL) &&
	    (basic_callback == NULL)) {
		error("Server does not work without authentication");
		return listener;
	}

	wsmand_shutdown_add_handler(listener_shutdown_handler,
				    &continue_working);

	if ((lsn = open_listening_port(port)) == -1) {
		error("cannot open port %d", port);
		exit(EXIT_FAILURE);
	}

#ifdef MULTITHREADED_SERVER
	if ((r = pthread_cond_init(&shttpd_cond, NULL)) != 0) {
		debug("pthread_cond_init failed = %d", r);
		return listener;
	}
	if ((r = pthread_mutex_init(&shttpd_mutex, NULL)) != 0) {
		debug("pthread_mutex_init failed = %d", r);
		return listener;
	}

	if ((r = pthread_attr_init(&pattrs)) != 0) {
		debug("pthread_attr_init failed = %d", r);
		return listener;
	}

	if ((r = pthread_attr_setdetachstate(&pattrs,
				     PTHREAD_CREATE_DETACHED)) != 0) {
		debug("pthread_attr_setdetachstate = %d", r);
		return listener;
	}

	request_list = list_create(LISTCOUNT_T_MAX);

	min_threads = wsmand_options_get_min_threads();
	max_threads = wsmand_options_get_max_threads();

	pthread_create(&thr_id, &pattrs,
		       wsman_server_auxiliary_loop_thread, cntx);
#ifdef ENABLE_EVENTING_SUPPORT
	pthread_create(&notificationManager_id, &pattrs, wsman_notification_manager, cntx);
#endif
	while (continue_working) {
		if ((sock = shttpd_accept(lsn, 1000)) == -1) {
			continue;
		}
		debug("Sock %d accepted", sock);
		node = lnode_create((void *) ((char *) NULL + sock));
		if (node == NULL) {
			debug("lnode_create == NULL");
			(void) shutdown(sock, 2);
			close(sock);
			continue;
		}

		pthread_mutex_lock(&shttpd_mutex);

		list_append(request_list, node);
		if (idle_threads > 0) {
			// we have idle thread waiting for a request
			debug("using idle thread. idle_threads = %d",
			      idle_threads);
			pthread_cond_signal(&shttpd_cond);
			pthread_mutex_unlock(&shttpd_mutex);
			continue;
		}
		if (num_threads >= max_threads) {
			// we have enough threads to serve requests
			debug("Using existing thread. %d > %d",
			      num_threads, max_threads);
			pthread_mutex_unlock(&shttpd_mutex);
			continue;
		}
		debug("Creating new thread. Old num_threads = %d",
		      num_threads);
		r = pthread_create(&thr_id, &pattrs, service_connection,
				   soap);
		if (r == 0) {
			num_threads++;
			debug("Thread %d created", thr_id);
			pthread_mutex_unlock(&shttpd_mutex);
			continue;
		}
		debug("pthread_create failed = %d. num_threads = %d",
		      r, num_threads);
		if (num_threads > 0) {
			// we have threads to serve request
			debug
			    ("we have threads to serve request. num_threads = %d",
			     num_threads);
			pthread_mutex_unlock(&shttpd_mutex);
			continue;
		}

		// So we couldn't create even one thread. Serve the request here
		debug("Serve on main thread");
		node = list_delete(request_list, node);
		if (node) {
			lnode_destroy(node);
		} else {
			error("Coundn't find node in a list");
		}

		pthread_mutex_unlock(&shttpd_mutex);

		handle_socket(sock, soap);
	}

	pthread_mutex_lock(&shttpd_mutex);
	while (num_threads > 0) {
		pthread_cond_broadcast(&shttpd_cond);
		timespec.tv_sec = 1;
		timespec.tv_nsec = 0;
		pthread_cond_timedwait(&shttpd_cond, &shttpd_mutex,
				       &timespec);
	}
	pthread_mutex_unlock(&shttpd_mutex);

#else
	ctx = create_shttpd_context(soap);
	if (ctx == NULL) {
		error("Could not create shttpd context");
		return listener;
	}
	shttpd_listen(ctx, lsn);
	while (continue_working) {
		shttpd_poll(ctx, 1000);
	}
	shttpd_fini(ctx);

#endif
	debug("shttpd_poll loop canceled");

	return listener;
}
