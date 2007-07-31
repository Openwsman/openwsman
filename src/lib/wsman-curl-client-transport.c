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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <curl/curl.h>
#include <curl/easy.h>

#include "u/libu.h"
#include "wsman-types.h"
#include "wsman-client.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-debug.h"
#include "wsman-client-transport.h"

#define DEFAULT_TRANSFER_LEN 32000

extern wsman_auth_request_func_t request_func;
void wsmc_handler( WsManClient *cl, WsXmlDocH rqstDoc, void* user_data);

static pthread_mutex_t curl_mutex = PTHREAD_MUTEX_INITIALIZER;


static long
reauthenticate(WsManClient *cl,
        long auth_set, long auth_avail, char **username, char **password)
{
	long choosen_auth = 0;
	wsman_auth_type_t ws_auth = WS_NO_AUTH;

	if (auth_avail &  CURLAUTH_GSSNEGOTIATE &&
			wsman_is_auth_method(cl, WS_GSSNEGOTIATE_AUTH)) {
		choosen_auth = CURLAUTH_GSSNEGOTIATE;
		ws_auth = WS_GSSNEGOTIATE_AUTH;
		goto REQUEST_PASSWORD;
	}
	if (auth_avail & CURLAUTH_DIGEST &&
			wsman_is_auth_method(cl, WS_DIGEST_AUTH)) {
		choosen_auth = CURLAUTH_DIGEST;
		ws_auth = WS_DIGEST_AUTH;
		goto REQUEST_PASSWORD;
	}
	if (auth_avail & CURLAUTH_NTLM &&
			wsman_is_auth_method(cl, WS_NTLM_AUTH)) {
		choosen_auth = CURLAUTH_NTLM;
		ws_auth = WS_NTLM_AUTH;
		goto REQUEST_PASSWORD;
	}
	if (auth_avail & CURLAUTH_BASIC &&
			wsman_is_auth_method(cl, WS_BASIC_AUTH)) {
		ws_auth = WS_BASIC_AUTH;
		choosen_auth = CURLAUTH_BASIC;
		goto REQUEST_PASSWORD;
	}

	printf("Client does not support authentication type "
			" acceptable by server\n");
	return 0;


REQUEST_PASSWORD:
	message("%s authentication is used",
			wsmc_transport_get_auth_name(ws_auth));
	if (auth_set == 0 && *username && *password) {
		// use existing username and password
		return choosen_auth;
	}

	if ( cl->authentication.auth_request_func )
		cl->authentication.auth_request_func(ws_auth, username, password);

	if (!(*username) || strlen(*username) == 0) {
		debug("No username. Authorization canceled");
		return 0;
	}
	return choosen_auth;
}


static WS_LASTERR_Code
convert_to_last_error(CURLcode r)
{
	switch (r) {
	case CURLE_OK:
		return WS_LASTERR_OK;
	case CURLE_FAILED_INIT:
		return WS_LASTERR_FAILED_INIT;
	case CURLE_UNSUPPORTED_PROTOCOL:
		return WS_LASTERR_UNSUPPORTED_PROTOCOL;
	case CURLE_URL_MALFORMAT:
		return WS_LASTERR_URL_MALFORMAT;
	case CURLE_COULDNT_RESOLVE_PROXY:
		return WS_LASTERR_COULDNT_RESOLVE_PROXY;
	case CURLE_COULDNT_RESOLVE_HOST:
		return WS_LASTERR_COULDNT_RESOLVE_HOST;
	case CURLE_COULDNT_CONNECT:
		return WS_LASTERR_COULDNT_CONNECT;
	case CURLE_HTTP_RETURNED_ERROR:
		return WS_LASTERR_HTTP_RETURNED_ERROR;
	case CURLE_WRITE_ERROR:
		return WS_LASTERR_WRITE_ERROR;
	case CURLE_READ_ERROR:
		return WS_LASTERR_READ_ERROR;
	case CURLE_OUT_OF_MEMORY:
		return WS_LASTERR_OUT_OF_MEMORY;
	case CURLE_OPERATION_TIMEOUTED:
		return WS_LASTERR_OPERATION_TIMEOUTED;
	case CURLE_HTTP_POST_ERROR:
		return WS_LASTERR_HTTP_POST_ERROR;
	case CURLE_BAD_DOWNLOAD_RESUME:
		return WS_LASTERR_BAD_DOWNLOAD_RESUME;
	case CURLE_TOO_MANY_REDIRECTS:
		return WS_LASTERR_TOO_MANY_REDIRECTS;
	case CURLE_SSL_CONNECT_ERROR:
		return WS_LASTERR_SSL_CONNECT_ERROR;
	case CURLE_SSL_PEER_CERTIFICATE:
		return WS_LASTERR_SSL_PEER_CERTIFICATE;
	case CURLE_SSL_ENGINE_NOTFOUND:
		return WS_LASTERR_SSL_ENGINE_NOTFOUND;
	case CURLE_SSL_ENGINE_SETFAILED:
		return WS_LASTERR_SSL_ENGINE_SETFAILED;
	case CURLE_SSL_CERTPROBLEM:
		return WS_LASTERR_SSL_CERTPROBLEM;
	case CURLE_SSL_CACERT:
		return WS_LASTERR_SSL_CACERT;
#if LIBCURL_VERSION_NUM > 0x70C01
	case CURLE_SSL_ENGINE_INITFAILED:
		return WS_LASTERR_SSL_ENGINE_INITFAILED;
#endif
	case CURLE_SEND_ERROR:
		return WS_LASTERR_SEND_ERROR;
	case CURLE_RECV_ERROR:
		return WS_LASTERR_RECV_ERROR;
	case CURLE_BAD_CONTENT_ENCODING:
		return WS_LASTERR_BAD_CONTENT_ENCODING;
#if LIBCURL_VERSION_NUM > 0x70C01
	case CURLE_LOGIN_DENIED:
		return WS_LASTERR_LOGIN_DENIED;
#endif
	default:
		return WS_LASTERR_OTHER_ERROR;
	}
	return WS_LASTERR_OTHER_ERROR;
}

static size_t
write_handler( void *ptr, size_t size, size_t nmemb, void *data)
{
	u_buf_t *buf = data;
	size_t len;

	len = size * nmemb;
	u_buf_append(buf, ptr, len);
	debug("write_handler: recieved %d bytes, all = %d\n", len, u_buf_len(buf));
	return len;
}

static void *
init_curl_transport(WsManClient *cl)
{
	CURL *curl;
	CURLcode r = CURLE_OK;
#define curl_err(str)  debug("Error = %d (%s); %s", \
		r, curl_easy_strerror(r), str);
	curl = curl_easy_init();
	if (curl == NULL) {
		r = CURLE_FAILED_INIT;
		curl_global_cleanup();
		debug("Could not init easy curl");
		goto DONE;
	}
	debug("cl->authentication.verify_peer: %d", cl->authentication.verify_peer );
	// verify peer
	r = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, cl->authentication.verify_peer);
	if (r != 0) {
		curl_err("curl_easy_setopt(CURLOPT_SSL_VERIFYPEER) failed");
		goto DONE;
	}

	// verify host
	r = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, cl->authentication.verify_host);
	if (r != 0) {
		curl_err("curl_easy_setopt(CURLOPT_SSL_VERIFYHOST) failed");
		goto DONE;
	}

	r = curl_easy_setopt(curl, CURLOPT_PROXY, cl->proxy_data.proxy);
	if (r != 0) {
		curl_err("Could notcurl_easy_setopt(curl, CURLOPT_PROXY, ...)");
		goto DONE;
	}
	r = curl_easy_setopt(curl, CURLOPT_TIMEOUT, cl->transport_timeout);
	if (r != 0) {
		curl_err("Could notcurl_easy_setopt(curl, CURLOPT_TIMEOUT, ...)");
		goto DONE;
	}

	r = curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, cl->proxy_data.proxy_auth);
	if (r != 0) {
		curl_err("Could notcurl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, ...)");
		goto DONE;
	}

	if (cl->authentication.capath) {
	r = curl_easy_setopt(curl, CURLOPT_CAPATH, cl->authentication.capath);
	if (r != 0) {
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_CAPATH, ..)");
		goto DONE;
	}
	}
	// cainfo
	if (cl->authentication.cainfo) {
	r = curl_easy_setopt(curl, CURLOPT_CAINFO, cl->authentication.cainfo);
	if (r != 0) {
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_CAINFO, ..)");
		goto DONE;
	}
	}
	// sslkey
	r = curl_easy_setopt(curl, CURLOPT_SSLKEY, cl->authentication.sslkey);
	if (r != 0) {
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_SSLKEY, ..)");
		goto DONE;
	}
	// sslcert
	r = curl_easy_setopt(curl, CURLOPT_SSLCERT, cl->authentication.sslcert );
	if (r != 0) {
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_SSLCERT, ..)");
		goto DONE;
	}



	return (void *)curl;
 DONE:
	cl->last_error = convert_to_last_error(r);
	curl_easy_cleanup(curl);
	return NULL;
#undef curl_err
}

void
wsmc_handler( WsManClient *cl,
		WsXmlDocH rqstDoc,
		void* user_data)
{
#define curl_err(str)  debug("Error = %d (%s); %s", \
		r, curl_easy_strerror(r), str); \
	http_code = 400
	WsManConnection *con = cl->connection;
	CURL *curl = NULL;
	CURLcode r;
	char *upwd = NULL;
	char *usag = NULL;
	struct curl_slist *headers=NULL;
	char *buf = NULL;
	int len;
	char *soapact_header = NULL;
	long http_code;
	long auth_avail = 0;

	if (!cl->initialized && wsmc_transport_init(cl, NULL)) {
		cl->last_error = WS_LASTERR_FAILED_INIT;
		return;
	}
	if (cl->transport == NULL) {
		cl->transport = init_curl_transport(cl);
	}
	curl = (CURL *)cl->transport;
	if (curl == NULL) {
		cl->last_error = WS_LASTERR_FAILED_INIT;
		return;
	}

	r = curl_easy_setopt(curl, CURLOPT_URL, cl->data.endpoint);
	if (r != CURLE_OK) {
		http_code = 400;
		cl->fault_string = strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_URL, ...)");
		goto DONE;
	}


	r = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_handler);
	if (r != CURLE_OK) {
		http_code = 400;
		cl->fault_string = strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ..)");
		goto DONE;
	}

	r = curl_easy_setopt(curl, CURLOPT_WRITEDATA, con->response);
	if (r != CURLE_OK) {
		http_code = 400;
		cl->fault_string = strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_WRITEDATA, ..)");
		goto DONE;
	}
	headers = curl_slist_append(headers,
			"Content-Type: application/soap+xml;charset=UTF-8");
	usag = malloc(12 + strlen(wsman_transport_get_agent(cl)) + 1);
	if (usag == NULL) {
		r = CURLE_OUT_OF_MEMORY;
		http_code = 400;
		cl->fault_string = strdup("Could not malloc memory");
		curl_err("Could not malloc memory");
		goto DONE;
	}

	sprintf(usag, "User-Agent: %s", wsman_transport_get_agent(cl));
	headers = curl_slist_append(headers, usag);

	/*
	   char *soapaction;
	   soapaction = ws_xml_get_xpath_value(rqstDoc, "/s:Envelope/s:Header/wsa:Action");
	   if (soapaction) {
	   soapact_header = malloc(12 + strlen(soapaction) + 1);
	   if (soapact_header) {
	   sprintf(soapact_header, "SOAPAction: %s", soapaction);
	   headers = curl_slist_append(headers, soapact_header);
	   }
	   u_free(soapaction);
	   }
	   */

	r = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if (r != CURLE_OK) {
		http_code = 400;
		cl->fault_string = strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_HTTPHEADER, ..)");
		goto DONE;
	}

	ws_xml_dump_memory_enc(rqstDoc, &buf, &len, "UTF-8");

	r = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
	if (r != CURLE_OK) {
		http_code = 400;
		cl->fault_string = strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ..)");
		goto DONE;
	}

	while (1) {
            if (cl->data.user && cl->data.pwd && cl->data.auth_set) {
            r = curl_easy_setopt(curl, CURLOPT_HTTPAUTH, cl->data.auth_set);
			if (r != CURLE_OK) {
				http_code = 400;
				cl->fault_string = strdup(curl_easy_strerror(r));
				curl_err("curl_easy_setopt(CURLOPT_HTTPAUTH) failed");
				goto DONE;
			}
			u_free(upwd);
			upwd = malloc(strlen(cl->data.user) + strlen(cl->data.pwd) + 2);
			if (!upwd) {
				r = CURLE_OUT_OF_MEMORY;
				http_code = 400;
				cl->fault_string = strdup("Could not malloc memory");
				curl_err("Could not malloc memory");
				goto DONE;
			}
			sprintf(upwd, "%s:%s", cl->data.user, cl->data.pwd);
			r = curl_easy_setopt(curl, CURLOPT_USERPWD, upwd);
			if (r != CURLE_OK) {
				http_code = 400;
				cl->fault_string = strdup(curl_easy_strerror(r));
				curl_err("curl_easy_setopt(curl, CURLOPT_USERPWD, ..) failed");
				goto DONE;
			}
		}

		if (wsman_debug_level_debugged(DEBUG_LEVEL_MESSAGE)) {
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		}
		r = curl_easy_perform(curl);
		if (r != CURLE_OK) {
			http_code = 400;
			cl->fault_string = strdup(curl_easy_strerror(r));
			curl_err("curl_easy_perform failed");
			goto DONE;
		}
		r = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (r != CURLE_OK) {
			http_code = 400;
			cl->fault_string = strdup(curl_easy_strerror(r));
			curl_err("curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed");
			goto DONE;
		}

		if (http_code != 401) {
			break;
		}
		// we are here because of authentication required
		r = curl_easy_getinfo(curl, CURLINFO_HTTPAUTH_AVAIL, &auth_avail);
		if (r != CURLE_OK) {
			http_code = 400;
			cl->fault_string = strdup(curl_easy_strerror(r));
			curl_err("curl_easy_getinfo(CURLINFO_HTTPAUTH_AVAIL) failed");
			goto DONE;
		}
    /*
     *  FIXME: Why are we freeing credentials here?
     *  if (cl->data.auth_set) {
			if (cl->data.user) {
				u_free(cl->data.user);
				cl->data.user = NULL;
			}
			if (cl->data.pwd) {
				u_free(cl->data.pwd);
				cl->data.pwd = NULL;
			}
      }*/
                cl->data.auth_set = reauthenticate(cl, cl->data.auth_set, auth_avail,
                        &cl->data.user, &cl->data.pwd);
                u_buf_clear(con->response);
                if (cl->data.auth_set == 0) {
                    // user wants to cancel authentication
		    //FIXME
#if LIBCURL_VERSION_NUM > 0x70C01
                    r = CURLE_LOGIN_DENIED;
#else
		    r = 1000;
#endif
                    curl_err("User didn't provide authentication data");
                    goto DONE;
                }
        }

DONE:
	cl->response_code = http_code;
	cl->last_error = convert_to_last_error(r);
	curl_slist_free_all(headers);
	u_free(soapact_header);
	u_free(usag);
	u_free(upwd);
#ifdef _WIN32
	xmlFree(buf);
#else
	u_free(buf);
#endif

	return;
#undef curl_err

}


int wsmc_transport_init(WsManClient *cl, void *arg)
{
	CURLcode r;

	pthread_mutex_lock(&curl_mutex);
	if (cl->initialized) {
		pthread_mutex_unlock(&curl_mutex);
		return 0;
	}
	r = curl_global_init(CURL_GLOBAL_SSL | CURL_GLOBAL_WIN32);
	if (r == CURLE_OK) {
		cl->initialized = 1;
	}
	pthread_mutex_unlock(&curl_mutex);
	if (r != CURLE_OK) {
		debug("Error = %d (%s); Could not initialize curl globals",
				r, curl_easy_strerror(r));
	}
	return (r == CURLE_OK ? 0 : 1);
}

void wsmc_transport_fini(WsManClient *cl)
{
	pthread_mutex_lock(&curl_mutex);
	if (!cl->initialized) {
		pthread_mutex_unlock(&curl_mutex);
		return;
	}
	curl_global_cleanup();
	cl->initialized = 0;
	pthread_mutex_unlock(&curl_mutex);

}

void
wsman_transport_close_transport(WsManClient *cl)
{
	if (cl->transport != NULL) {
		curl_easy_cleanup((CURL *)cl->transport);
	}
	cl->transport = NULL;
}


