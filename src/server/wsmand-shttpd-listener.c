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
 */

#define _GNU_SOURCE
#include "wsman_config.h"


#include <stdlib.h> 
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>



#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"

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

#include <pthread.h>
#include <sys/socket.h>

static pthread_mutex_t      shttpd_mutex;
static pthread_cond_t       shttpd_cond;
static list_t               *request_list;
static int                  num_threads = 0;
static int                  max_threads = 4;
static int                  min_threads = 2;
static int                  idle_threads = 0;

#endif

static int continue_working = 1;
static int (*basic_auth_callback)(char *, char *) = NULL;


static int
digest_auth_callback(char *realm, char *method, struct digest *dig)
{
    WSmanAuthDigest wsdig;
    char *filename = wsmand_options_get_digest_password_file();

    if (filename == NULL) {
        debug( "Could not get digest password file name");
        return 0;
    }       
    wsdig.request_method = method;
    wsdig.username       = dig->user;
    wsdig.realm          = realm;
    wsdig.digest_uri     = dig->uri;
    wsdig.nonce          = dig->nonce;
    wsdig.cnonce         = dig->cnonce;
    wsdig.qop            = dig->qop;
    strncpy(wsdig.nonce_count, dig->nc, sizeof (wsdig.nonce_count));
    wsdig.digest_response = dig->resp;

    return ws_authorize_digest(filename, &wsdig);
}


#if 0
static int
basic_auth_callback(char *username, char *password)
{
        return ws_authorize_basic(username, password);
}
#endif


static char *
shttp_reason_phrase(int status) {
        return "Error";
}

typedef struct {
      char *response;
      int length;
      int ind;
}   ShttpMessage;



static int server_callback (struct shttpd_arg_t *arg)
{
    const char *path, *method;
    char *default_path;
    const char *content_type;
    const char *encoding;

    int status = WSMAN_STATUS_OK;

    ShttpMessage *shttp_msg = (ShttpMessage *)arg->state;
    WsmanMessage *wsman_msg;
    int n = 0;
    int k;

    if (shttp_msg != NULL) {
        // WE already have the response. Just continue to send it to server
        goto CONTINUE;
    }

    // Here we must handle the initial request

    wsman_msg = wsman_soap_message_new();

    // Check HTTP headers
    path = shttpd_get_uri(arg);
    method = shttpd_get_env(arg, "REQUEST_METHOD");
    debug("%s %s HTTP/1.%d", method, path,
            shttpd_get_http_version(arg));

//    soup_message_foreach_header (msg->request_headers, print_header, NULL);
//    if (msg->request.length) {
//        debug("Request: %.*s", msg->request.length, msg->request.body);
//    }

    // Check Method
    if (strncmp(method, "POST", 4)) {
        debug( "Insupported method %s", method);
        status = WSMAN_STATUS_NOT_IMPLEMENTED;
        goto DONE;
    }

    default_path = wsmand_options_get_service_path();
    if (path) {
        if (strcmp(path, default_path) != 0 ) {
            status = WSMAN_STATUS_BAD_REQUEST;
            goto DONE;
        }
    } else {
        path = u_strdup ("");
    }

    content_type = shttpd_get_header(arg, "Content-Type");
    if (content_type && strncmp(content_type, SOAP_CONTENT_TYPE, strlen(SOAP_CONTENT_TYPE)) != 0 ) {
        status = WSMAN_STATUS_BAD_REQUEST;
        goto DONE;
    }
    encoding = strchr(content_type, '=') + 1;
    debug("Encoding: %s", encoding);


    env_t* fw = (env_t*)arg->user_data;	
    wsman_msg->status.fault_code = WSMAN_RC_OK;

    wsman_msg->request.length = shttpd_get_post_query_len(arg);
    wsman_msg->request.body   = (char *)malloc(wsman_msg->request.length);
    if (wsman_msg->request.body == NULL) {
        status = WSMAN_STATUS_INTERNAL_SERVER_ERROR;
        goto DONE;
    }
    (void) shttpd_get_post_query(arg, wsman_msg->request.body,
                    wsman_msg->request.length);    

    /*
    wsman_msg->auth_data.username = soup_server_auth_get_user(context->auth);
    wsman_msg->auth_data.password = context->auth->basic.passwd;
    */

    // Call dispatcher
    dispatch_inbound_call(fw, wsman_msg);

    if (wsman_msg->request.body) {
        free(wsman_msg->request.body);
        wsman_msg->request.body = NULL;
    }
    wsman_msg->request.length = 0;

    if ( wsman_fault_occured(wsman_msg) ) {
        char *buf;
        int  len;    		
        if (wsman_msg->in_doc != NULL) {
            wsman_generate_fault_buffer(
                    fw->cntx, 
                    wsman_msg->in_doc, 
                    wsman_msg->status.fault_code , 
                    wsman_msg->status.fault_detail_code, 
                    wsman_msg->status.fault_msg, 
                    &buf, &len);
            shttp_msg = (ShttpMessage *)malloc(sizeof (ShttpMessage));
            if (shttp_msg) {
                shttp_msg->length = len;
                shttp_msg->response = strndup(buf, len);
                shttp_msg->ind = 0;
            } else {
                //  XXX handle error
            }  
            free(buf);
        }

        status = WSMAN_STATUS_INTERNAL_SERVER_ERROR; // ?????
        goto DONE;
    }

    shttp_msg = (ShttpMessage *)malloc(sizeof (ShttpMessage));
    if (shttp_msg) {
        shttp_msg->length = wsman_msg->response.length;
        shttp_msg->response = (char *)wsman_msg->response.body;
        shttp_msg->ind = 0;
        status =wsman_msg->http_code;
    } else {
        status = WSMAN_STATUS_INTERNAL_SERVER_ERROR;
    }

    if (wsman_msg->in_doc != NULL) {
        ws_xml_destroy_doc(wsman_msg->in_doc);
    }
DONE:

    u_free(wsman_msg);
    debug("Response (status) %d", status, shttp_reason_phrase(status));

    // Here we begin to create response
    // we consider output buffer is large enough to hold all headers
    switch (status) {
        case WSMAN_STATUS_OK:
                n += snprintf(arg->buf + n, arg->buflen -n,
	               "HTTP/1.1 200 OK\r\n");
                break;
        default:
	       n += snprintf(arg->buf + n, arg->buflen -n, "HTTP/1.1 %d %s\r\n",
                        status, shttp_reason_phrase(status));
        break;
    }
    n += snprintf(arg->buf + n, arg->buflen -n, "Server: %s/%s\r\n",
                PACKAGE, VERSION);
    if (shttp_msg && shttp_msg->length > 0) {
        n += snprintf(arg->buf + n, arg->buflen -n,
                "Content-Type: %s\r\n",
                SOAP1_2_CONTENT_TYPE);
        n += snprintf(arg->buf + n, arg->buflen -n,
             "Content-Length: %d\r\n", shttp_msg->length);
    }
    n += snprintf(arg->buf + n, arg->buflen -n, "Connection: close\r\n");

    if (!shttp_msg || shttp_msg->length == 0) {
        n += snprintf(arg->buf + n, arg->buflen -n, "\r\n");
        arg->last = 1;
        if (shttp_msg) free(shttp_msg);
        return n;
    }

    n += snprintf(arg->buf + n, arg->buflen -n, "\r\n");

CONTINUE:
    k = arg->buflen - n;
    if (k <= shttp_msg->length - shttp_msg->ind) {
        // not enogh room for all message. transfer only part
        memcpy(arg->buf + n, shttp_msg->response + shttp_msg->ind, k);
        shttp_msg->ind += k;
        arg->state = shttp_msg;
        return n + k;
    }
    // Enough room for all message
    memcpy(arg->buf + n, shttp_msg->response + shttp_msg->ind,
                                shttp_msg->length - shttp_msg->ind);
    n += shttp_msg->length - shttp_msg->ind;
    if (n + 4 > arg->buflen) {
        // not enough room empty lines at the end of message
        arg->state = shttp_msg;
        shttp_msg->ind = shttp_msg->length;
        return n;
    }

    // here we can complete
    n += snprintf(arg->buf + n, arg->buflen -n, "\r\n\r\n");
    debug("%s", arg->buf);
    u_free(shttp_msg->response);
    u_free(shttp_msg);
    arg->last =1;
    arg->state = NULL;
    return n;
}



static void
listener_shutdown_handler(void* p)
{
        int *a = (int *)p;
        debug("listener_shutdown_handler started");
        *a = 0;
}


static struct shttpd_ctx  *
create_shttpd_context(SoapH soap)
{
    struct shttpd_ctx   *ctx;
    if (wsmand_options_get_ssl_cert_file() &&
                wsmand_options_get_ssl_key_file() &&
                (wsmand_options_get_server_ssl_port() > 0)) {
        message("Using SSL");
        ctx = shttpd_init(NULL,
            "ssl_certificate", wsmand_options_get_ssl_cert_file(),
            "ssl_priv_key", wsmand_options_get_ssl_key_file(),
            "auth_realm", AUTHENTICATION_REALM,
            "debug", wsmand_options_get_debug_level() > 0 ? "1" : "0",
            NULL);
//            port = wsmand_options_get_server_ssl_port();
    } else {
        ctx = shttpd_init(NULL,
            "auth_realm", AUTHENTICATION_REALM,
            "debug", wsmand_options_get_debug_level() > 0 ? "1" : "0",
            NULL);
    }
    if (ctx == NULL) {
        return NULL;
    }
    shttpd_register_url(ctx, wsmand_options_get_service_path(),
                                    server_callback, (void *) soap);

    if (wsmand_options_get_digest_password_file()) {
       shttpd_register_dauth_callback(ctx, digest_auth_callback);
        debug( "Using Digest Authorization");
    }
    if (basic_auth_callback) {
        shttpd_register_bauth_callback(ctx, basic_auth_callback);
        debug( "Using Basic Authorization %s",
            wsmand_option_get_basic_authenticator() ?
            wsmand_option_get_basic_authenticator() :
            wsmand_default_basic_authenticator());
    }

    return ctx;
}

#ifdef MULTITHREADED_SERVER

static void handle_socket(int sock,  SoapH soap)
{
    struct shttpd_ctx   *ctx;

    debug("Thread %d handles sock %d", pthread_self(), sock);

    ctx = create_shttpd_context(soap);
    if (ctx == NULL) {
        (void) shutdown (sock, 2);
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
    SoapH soap = (SoapH)arg;
//    struct timespec timespec;

    debug("shttpd thread %d started. num_threads = %d",
                pthread_self(), num_threads);
    pthread_mutex_lock(&shttpd_mutex);
    while (continue_working) {
        if (list_isempty(request_list)) {
            // no sockets to serve 
            if (num_threads > min_threads) {
                debug("we have too many threads %d > %d"
                      " Thread %d is being completed",
                            num_threads, min_threads, pthread_self());
                break;
            } else {
                idle_threads++;
//                timespec.tv_sec = 1;
//                timespec.tv_nsec = 0;
                debug("Thread %d goes to idle state", pthread_self());
                (void) pthread_cond_wait(&shttpd_cond, &shttpd_mutex);
                idle_threads--;
                continue;
            }
        }
        node = list_del_first(request_list);
        sock = (int)lnode_get(node);
        pthread_mutex_unlock(&shttpd_mutex);
        lnode_destroy(node);
        handle_socket(sock,soap);
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


static int
initialize_basic_authenticator(void)
{
    char *auth;
    char *arg;
    void *hnd;
    int (*init)(char *);
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
        arg  = wsmand_options_get_basic_password_file();
    } else {
        auth = wsmand_option_get_basic_authenticator();
        arg  = wsmand_option_get_basic_authenticator_arg();
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
        fprintf(stderr, "Could not resolve authorize() in %s\n", name);
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


WsManListenerH*
wsmand_start_server(dictionary *ini) 
{
    WsManListenerH *listener = wsman_dispatch_list_new();
    listener->config = ini;
    WsContextH cntx = wsman_init_plugins(listener);
#ifdef MULTITHREADED_SERVER
    int r;
    int sock;
    lnode_t     *node;
    pthread_t   thr_id;
    pthread_attr_t  pattrs;
    struct timespec timespec;
#else
    struct shttpd_ctx   *ctx;

#endif

    if (cntx == NULL) {
        return NULL;
    }
    SoapH soap = ws_context_get_runtime(cntx);

    if (initialize_basic_authenticator()) {
        return NULL;
    }

    int lsn;
    int port;


    if (wsmand_options_get_ssl_cert_file() &&
                wsmand_options_get_ssl_key_file() &&
                (wsmand_options_get_server_ssl_port() > 0)) {
        port = wsmand_options_get_server_ssl_port();
        message("Using SSL");
    } else {
        port = wsmand_options_get_server_port();
    }
    if (port == 0) {
        port = 9001;
    }
    message( "     Working on port %d", port);
    if (wsmand_options_get_digest_password_file()) {
        message( "Using Digest Authorization");
    }
    if (basic_auth_callback) {
        message( "Using Basic Authorization %s",
            wsmand_option_get_basic_authenticator() ?
            wsmand_option_get_basic_authenticator() :
            wsmand_default_basic_authenticator());
    }

    wsmand_shutdown_add_handler(listener_shutdown_handler, &continue_working);

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

    request_list = list_create(-1);

    min_threads = wsmand_options_get_min_threads();
    max_threads = wsmand_options_get_max_threads();

    while( continue_working) {
        if ((sock = shttpd_accept(lsn, 1000)) == -1) {
            continue;
        }
        debug("Sock %d accepted", sock);
        node = lnode_create((void *)sock);
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
            debug("using idle thread. idle_threads = %d", idle_threads);
            pthread_cond_signal(&shttpd_cond);
            pthread_mutex_unlock(&shttpd_mutex);
            continue;
        }
        if (num_threads >=  max_threads) {
            // we have enough threads to serve requests 
            debug("Using existing thread. %d > %d", num_threads, max_threads);
            pthread_mutex_unlock(&shttpd_mutex);
            continue;
        }
        debug("Creating new thread. Old num_threads = %d", num_threads);
        r = pthread_create(&thr_id, &pattrs, service_connection, soap);
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
            debug("we have threads to serve request. num_threads = %d",
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
    debug( "shttpd_poll loop canceled");

    return listener;
}


