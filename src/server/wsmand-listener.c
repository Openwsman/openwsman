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

#define OPENWSMAN
#include "shttpd.h"

#include "wsman-plugins.h"
#include "wsmand-listener.h"
#include "wsmand-daemon.h"
#include "wsmand-auth.h"
#include "wsman-server.h"
#include "wsman-plugins.h"

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
static int (*basic_auth_callback) (char *, char *) = NULL;


static int
digest_auth_callback(char *realm, char *method, struct digest *dig)
{
	WSmanAuthDigest wsdig;
	char *filename = wsmand_options_get_digest_password_file();

	if (filename == NULL) {
		debug("Could not get digest password file name");
		return 0;
	}
	wsdig.request_method = method;
	wsdig.username = dig->user;
	wsdig.realm = realm;
	wsdig.digest_uri = dig->uri;
	wsdig.nonce = dig->nonce;
	wsdig.cnonce = dig->cnonce;
	wsdig.qop = dig->qop;
	strncpy(wsdig.nonce_count, dig->nc, sizeof(wsdig.nonce_count));
	wsdig.digest_response = dig->resp;

	return ws_authorize_digest(filename, &wsdig);
}

static char *shttp_reason_phrase(int status)
{
	if (status == WSMAN_STATUS_OK) {
		return "OK";
	}
	return "Error";
}


typedef struct {
	char *response;
	int length;
	int ind;
} ShttpMessage;



static int server_callback(struct shttpd_arg_t *arg)
{
	const char *method;
	const char *content_type;
//    char *default_path;
//    const char *path;
//    const char *encoding;
	int status = WSMAN_STATUS_OK;
	char *fault_reason = NULL;

	ShttpMessage *shttp_msg = (ShttpMessage *) arg->state;
	int n = 0;
	int k;

	debug("Server callback started %s. len = %d, sent = %d",
	      (shttp_msg == NULL) ? "initialy" : "continue",
	      arg->buflen, (shttp_msg == NULL) ? 0 : shttp_msg->ind);
	if (shttp_msg != NULL) {
		// We already have the response, but server
		// output buffer is smaller then it.
		// Some part of resopnse have already sent.
		// Just continue to send it to server
		goto CONTINUE;
	}
	// Here we must handle the initial request
	WsmanMessage *wsman_msg = wsman_soap_message_new();

	// Check HTTP headers

	method = shttpd_get_env(arg, "REQUEST_METHOD");
	if (strncmp(method, "POST", 4)) {
		debug("Unsupported method %s", method);
		status = WSMAN_STATUS_METHOD_NOT_ALLOWED;
		fault_reason = "POST method supported only";
	}


	content_type = shttpd_get_header(arg, "Content-Type");
	if (content_type && strncmp(content_type,
				    SOAP_CONTENT_TYPE,
				    strlen(SOAP_CONTENT_TYPE)) != 0) {
		status = WSMAN_STATUS_UNSUPPORTED_MEDIA_TYPE;
		fault_reason = "Unsupported content type";
		goto DONE;
	}

	SoapH soap = (SoapH) arg->user_data;
	wsman_msg->status.fault_code = WSMAN_RC_OK;
	wsman_msg->http_headers = shttpd_get_all_headers(arg);

	// Get request from http server
	size_t length = shttpd_get_post_query_len(arg);
	char *body = shttpd_get_post_query(arg);
	if (body == NULL) {
		status = WSMAN_STATUS_BAD_REQUEST;
		fault_reason = "No request body";
		error("NULL request body. len = %d", length);
	}
	else {
		debug("Posted request: %s", body);
	}
	u_buf_construct(wsman_msg->request, body, length, length);

	// some plugins can use credentials for its
	// own authentication
	shttpd_get_credentials(arg, &wsman_msg->auth_data.username,
			       &wsman_msg->auth_data.password);


	// Call dispatcher. Real request handling
	if (status == WSMAN_STATUS_OK) {
		// dispatch if we didn't find out the error
		dispatch_inbound_call(soap, wsman_msg, NULL);
		status = wsman_msg->http_code;
	}


	if (wsman_msg->request) {
		// we don't need request any more
		(void) u_buf_steal(wsman_msg->request);
		u_buf_free(wsman_msg->request);
		wsman_msg->request = NULL;
	}
	// here we start to handle the response

	shttp_msg = (ShttpMessage *) malloc(sizeof(ShttpMessage));
	if (shttp_msg == NULL) {
		status = WSMAN_STATUS_INTERNAL_SERVER_ERROR;
		fault_reason = "No memory";
		goto DONE;
	}


	shttp_msg->length = u_buf_len(wsman_msg->response);
	debug("message len = %d", shttp_msg->length);
	shttp_msg->response = u_buf_steal(wsman_msg->response);
	shttp_msg->ind = 0;

      DONE:

	if (wsman_msg->response) {
		u_buf_free(wsman_msg->response);
		wsman_msg->response = NULL;
	}
	//   wsman_soap_message_destroy(wsman_msg);
	if (wsman_msg->http_headers) {
		hash_free(wsman_msg->http_headers);
	}
	u_free(wsman_msg);
	if (fault_reason == NULL) {
		fault_reason = shttp_reason_phrase(status);
	}
	debug("Response (status) %d (%s)", status, fault_reason);

	// Here we begin to create the http response.
	// Create the headers at first.
	// We consider output buffer of server is large enough to hold all headers.

	n += snprintf(arg->buf + n, arg->buflen - n, "HTTP/1.1 %d %s\r\n",
		      status, fault_reason);
	n += snprintf(arg->buf + n, arg->buflen - n, "Server: %s/%s\r\n",
		      PACKAGE, VERSION);
/*
    if (status != WSMAN_STATUS_OK) {
        n += snprintf(arg->buf + n, arg->buflen -n, "\r\n%d %s\r\n",
                status, fault_reason);
        arg->last = 1;
        u_free(shttp_msg);
        return n;
    }
*/
	if (!shttp_msg || shttp_msg->length == 0) {
		// can't send the body of response or nothing to send
		n += snprintf(arg->buf + n, arg->buflen - n, "\r\n");
		arg->last = 1;
		u_free(shttp_msg);
		return n;
	}

	n += snprintf(arg->buf + n, arg->buflen - n,
		      "Content-Type: %s\r\n", SOAP1_2_CONTENT_TYPE);
	n += snprintf(arg->buf + n, arg->buflen - n,
		      "Content-Length: %d\r\n", shttp_msg->length);
	n += snprintf(arg->buf + n, arg->buflen - n, "\r\n");

	// add response body to output buffer
      CONTINUE:
	k = arg->buflen - n;
	if (k <= shttp_msg->length - shttp_msg->ind) {
		// not enogh room for all message. transfer only part
		memcpy(arg->buf + n, shttp_msg->response + shttp_msg->ind,
		       k);
		shttp_msg->ind += k;
		arg->state = shttp_msg;
		return n + k;
	}
	// Enough room for all response body
	memcpy(arg->buf + n, shttp_msg->response + shttp_msg->ind,
	       shttp_msg->length - shttp_msg->ind);
	n += shttp_msg->length - shttp_msg->ind;
	if (n + 4 > arg->buflen) {
		// not enough room for empty lines at the end of the message
		arg->state = shttp_msg;
		shttp_msg->ind = shttp_msg->length;
		return n;
	}
	// here we can complete
	n += snprintf(arg->buf + n, arg->buflen - n, "\r\n\r\n");
	debug("%s", arg->buf);
	u_free(shttp_msg->response);
	u_free(shttp_msg);

	arg->last = 1;
	arg->state = NULL;
	return n;
}

static void wsmand_start_notification_manager(WsContextH cntx, SubsRepositoryEntryH entry, int subsNum)
{
	WsmanMessage *wsman_msg = wsman_soap_message_new();
	if(wsman_msg == NULL) return;
	char *strdoc = entry->strdoc;
	u_buf_construct(wsman_msg->request, strdoc, strlen(strdoc)+1, strlen(strdoc)+1);
	dispatch_inbound_call(cntx->soap, wsman_msg, NULL);
	wsman_soap_message_destroy(wsman_msg);
	if(list_count(cntx->soap->subscriptionMemList) > subsNum) {
		lnode_t *node = list_last(cntx->soap->subscriptionMemList);
		WsSubscribeInfo *subs = (WsSubscribeInfo *)node->list_data;
		//Delete new subscription file coz in fact we've got it
		cntx->soap->subscriptionOpSet->delete_subscription(cntx->soap->uri_subsRepository, subs->subsId);
		//Update UUID in the memory
		strncpy(subs->subsId, entry->uuid+5, EUIDLEN);
	}
}

static void listener_shutdown_handler(void *p)
{
	int *a = (int *) p;
	debug("listener_shutdown_handler started");
	*a = 0;
}


static struct shttpd_ctx *create_shttpd_context(SoapH soap)
{
	struct shttpd_ctx *ctx;
	if (wsmand_options_get_use_ssl()) {
		message("Using SSL");
		ctx = shttpd_init(NULL,
				  "ssl_certificate",
				  wsmand_options_get_ssl_cert_file(),
				  "ssl_priv_key",
				  wsmand_options_get_ssl_key_file(),
				  "auth_realm", AUTHENTICATION_REALM,
				  "debug",
				  wsmand_options_get_debug_level() >
				  0 ? "1" : "0", NULL);
	} else {
		ctx = shttpd_init(NULL,
				  "auth_realm", AUTHENTICATION_REALM,
				  "debug",
				  wsmand_options_get_debug_level() >
				  0 ? "1" : "0", NULL);
	}
	if (ctx == NULL) {
		return NULL;
	}
	shttpd_register_url(ctx, wsmand_options_get_service_path(),
			    server_callback, 0, (void *) soap);
	if (wsmand_options_get_digest_password_file()) {
		shttpd_register_dauth_callback(ctx, digest_auth_callback);
		debug("Using Digest Authorization");
	}
	if (basic_auth_callback) {
		shttpd_register_bauth_callback(ctx, basic_auth_callback);
		debug("Using Basic Authorization %s",
		      wsmand_option_get_basic_authenticator()?
		      wsmand_option_get_basic_authenticator() :
		      wsmand_default_basic_authenticator());
	}

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
	shttpd_add(ctx, sock);
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
	basic_auth_callback = dlsym(hnd, "authorize");
	if (basic_auth_callback == NULL) {
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
	SubsRepositoryOpSetH ops = wsman_init_subscription_repository(cntx, wsmand_options_get_subscription_repository_uri());
	list_t *subs_list = list_create(-1);
	debug("subscription_repository_uri = %s", wsmand_options_get_subscription_repository_uri());
	if(ops->load_subscription(wsmand_options_get_subscription_repository_uri(), subs_list) == 0) {
		lnode_t *node = list_first(subs_list);
		while(node) {
			SubsRepositoryEntryH entry = (SubsRepositoryEntryH)node->list_data;
			debug("load subscription %s", entry->uuid);
			wsmand_start_notification_manager(cntx, entry, list_count(cntx->soap->subscriptionMemList));
			u_free(entry->uuid);
			u_free(entry);
			list_delete(subs_list, node);
			lnode_destroy(node);
			node = list_first(subs_list);
		}
	}
	list_destroy(subs_list);
#ifdef MULTITHREADED_SERVER
	int r;
	int sock;
	lnode_t *node;
	pthread_t thr_id;
	pthread_t notificationManager_id;
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
	ws_set_context_enumIdleTimeout(cntx,
				       wsmand_options_get_enumIdleTimeout
				       ());
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
	if (basic_auth_callback) {
		message("Using Basic Authorization %s",
			wsmand_option_get_basic_authenticator()?
			wsmand_option_get_basic_authenticator() :
			wsmand_default_basic_authenticator());
	}

	if ((wsmand_options_get_digest_password_file() == NULL) &&
	    (basic_auth_callback == NULL)) {
		error("Server does not work without authentication");
		return listener;
	}

	wsmand_shutdown_add_handler(listener_shutdown_handler,
				    &continue_working);

	lsn = shttpd_open_port(port);

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

	pthread_create(&notificationManager_id, &pattrs, wsman_notification_manager, cntx);

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
