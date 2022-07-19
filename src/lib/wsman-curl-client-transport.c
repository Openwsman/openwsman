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

#ifdef ENABLE_EVENTING_SUPPORT
#include <openssl/opensslv.h>
#include <openssl/ssl.h>
#endif

#include "u/libu.h"
#include "wsman-types.h"
#include "wsman-client.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-debug.h"
#include "wsman-client-transport.h"

#define DEFAULT_TRANSFER_LEN 32000

#ifndef CURLOPT_CRLFILE
	#define CURLOPT_CRLFILE 10169
#endif
#ifndef CURLE_SSL_CRL_BADFILE
	#define CURLE_SSL_CRL_BADFILE 82
#endif


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

	debug("Client does not support authentication type 0x%04x"
			" acceptable by server\n", auth_avail);
	return 0;


REQUEST_PASSWORD:
	message("%s authentication is used",
			wsmc_transport_get_auth_name(ws_auth));
	if (auth_set == 0 && *username && *password) {
		// use existing username and password
		return choosen_auth;
	}

	if (cl->authentication.auth_request_func) {
		debug("Invoking Auth request callback");

		/* free the username and password as these are OUT params to the auth_request_func
		 * which the caller is expected to set
		 */
		if (*username) {
			u_free(*username);
			*username = NULL;
		}
		if (*password) {
			u_free(*password);
			*password = NULL;
		}
		cl->authentication.auth_request_func(cl, ws_auth, username, password);
	}
	else
		return 0;


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
        case CURLE_BAD_FUNCTION_ARGUMENT:
                return WS_LASTERR_CURL_BAD_FUNCTION_ARG;
#if LIBCURL_VERSION_NUM < 0x073E00
	case CURLE_SSL_PEER_CERTIFICATE:
		return WS_LASTERR_SSL_PEER_CERTIFICATE;
#endif
	case CURLE_SSL_ENGINE_NOTFOUND:
		return WS_LASTERR_SSL_ENGINE_NOTFOUND;
	case CURLE_SSL_ENGINE_SETFAILED:
		return WS_LASTERR_SSL_ENGINE_SETFAILED;
	case CURLE_SSL_CERTPROBLEM:
		return WS_LASTERR_SSL_CERTPROBLEM;
#if LIBCURL_VERSION_NUM < 0x073E00
	case CURLE_SSL_CACERT:
		return WS_LASTERR_SSL_CACERT;
#else
	case CURLE_PEER_FAILED_VERIFICATION:
		return WS_LASTERR_SSL_PEER_CERTIFICATE;
#endif
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
#if LIBCURL_VERSION_NUM >= 0x70D01
	case CURLE_LOGIN_DENIED:
		return WS_LASTERR_LOGIN_DENIED;
#else
	/* Map 67 (same as CURLE_LOGIN_DENIED) got from OS that has lower version of CURL in which
	 * CURLE_LOGIN_DENIED is not defined. This way we get the same error code in case of login failure
	 */
	case 67:
		return WS_LASTERR_LOGIN_DENIED;
#endif
	case CURLE_SSL_CRL_BADFILE:
		return WS_LASTERR_BAD_CRL_FILE;

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
	debug("write_handler: received %d bytes, all = %d\n", len, u_buf_len(buf));
	return len;
}

#ifdef ENABLE_EVENTING_SUPPORT
static int ssl_certificate_thumbprint_verify_callback(X509_STORE_CTX *ctx, void *arg)
{
	unsigned char *thumbprint = (unsigned char *)arg;
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
        X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
#else
	X509 *cert = ctx->cert;
#endif
	EVP_MD                                  *tempDigest;

	unsigned char   tempFingerprint[EVP_MAX_MD_SIZE];
	unsigned int      tempFingerprintLen;
	tempDigest = (EVP_MD*)EVP_sha1( );
	if ( X509_digest(cert, tempDigest, tempFingerprint, &tempFingerprintLen ) <= 0)
		return 0;
	if(!memcmp(tempFingerprint, thumbprint, tempFingerprintLen))
		return 1;
	return 0;
}

static CURLcode
sslctxfun(CURL *curl, void *sslctx, void *parm)
{
	CURLcode r = CURLE_OK;
	SSL_CTX * ctx = (SSL_CTX *) sslctx ;
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER |  SSL_VERIFY_FAIL_IF_NO_PEER_CERT, 0 );
	SSL_CTX_set_cert_verify_callback(ctx, ssl_certificate_thumbprint_verify_callback, parm);
	return r;
}
#endif

static void *
init_curl_transport(WsManClient *cl)
{
	CURL *curl;
	CURLcode r = CURLE_OK;
        char *sslhack;
        long sslversion;
        dictionary *ini = NULL;
#ifndef _WIN32
        ini = iniparser_new(wsmc_get_conffile(cl));
#endif

#define curl_err(str)  debug("Error = %d (%s); %s", \
		r, curl_easy_strerror(r), str);
	curl = curl_easy_init();
	if (curl == NULL) {
		r = CURLE_FAILED_INIT;
		debug("Could not init easy curl");
		goto DONE;
	}
        // client:curlopt_nosignal
        if (ini) {
          int nosignal = iniparser_getint(ini, "client:curlopt_nosignal", 0);
          r = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, nosignal);
          if (r != 0) {
            curl_err("curl_easy_setopt(CURLOPT_NOSIGNAL) failed");
            goto DONE;
          }
        }
	debug("cl->authentication.verify_peer: %d", cl->authentication.verify_peer );
	// verify peer
	r = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, wsman_transport_get_verify_peer(cl)?1:0);
	if (r != 0) {
		curl_err("curl_easy_setopt(CURLOPT_SSL_VERIFYPEER) failed");
		goto DONE;
	}

	// verify host
	r = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, wsman_transport_get_verify_host(cl)?2:0);
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
	
	if (0 != cl->authentication.verify_peer && 0 != cl->authentication.crl_check)
	{		
		if (cl->authentication.crl_file == NULL)
		{
			if (ini != NULL)
			{
			        char *crlfile = iniparser_getstr(ini, "client:crlfile");
				wsman_transport_set_crlfile(cl, crlfile);
			}
		}
		if (cl->authentication.crl_file != NULL)
		{
			debug("wsman-curl-client-transport.c: init_curl_transport() : CRL file = %s\n",cl->authentication.crl_file);
			r = curl_easy_setopt(curl, CURLOPT_CRLFILE, cl->authentication.crl_file);		
		}
		
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
	// certificate thumbprint
#ifdef ENABLE_EVENTING_SUPPORT
/*  Bug in e.g. Fedora: [ curl-Bugs-1924441 ] SSL callback option with NSS-linked libcurl */
#ifndef NO_SSL_CALLBACK
	else if (strlen((char *)cl->authentication.certificatethumbprint) > 0 && 0 != cl->authentication.verify_peer) {
		r = curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, sslctxfun);
		if(r != 0) {
			curl_err("Could not curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION)");
			goto DONE;
		}
		r = curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA, (void *)cl->authentication.certificatethumbprint);
		if(r != 0) {
			curl_err("Could not curl_easy_setopt(curl, CURLOPT_SSL_CTX_DATA)");
			goto DONE;
		}
	}
#endif
#endif
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

        /* enforce specific ssl version if requested */
        sslhack = getenv("OPENWSMAN_CURL_TRANSPORT_SSLVERSION");
        if (sslhack == NULL) {
          sslversion = CURL_SSLVERSION_DEFAULT;
        } else if (!strcmp(sslhack,"tlsv1")) {
          sslversion = CURL_SSLVERSION_TLSv1;
        } else if (!strcmp(sslhack,"sslv2")) {
          sslversion = CURL_SSLVERSION_SSLv2;
        } else if (!strcmp(sslhack,"sslv3")) {
          sslversion = CURL_SSLVERSION_SSLv3;
#if LIBCURL_VERSION_NUM >= 0x072200
        } else if (!strcmp(sslhack,"tlsv1.0")) {
          sslversion = CURL_SSLVERSION_TLSv1_0;
        } else if (!strcmp(sslhack,"tlsv1.1")) {
          sslversion = CURL_SSLVERSION_TLSv1_1;
        } else if (!strcmp(sslhack,"tlsv1.2")) {
          sslversion = CURL_SSLVERSION_TLSv1_2;
#endif
        }
        else {
          sslversion = CURL_SSLVERSION_DEFAULT;
        }
        r = curl_easy_setopt(curl, CURLOPT_SSLVERSION, sslversion );
        if (r != 0) {
          curl_err("Could not curl_easy_setopt(curl, CURLOPT_SSLVERSION, ..)");
          goto DONE;
        }

        iniparser_free(ini);
	return (void *)curl;
 DONE:
	cl->last_error = convert_to_last_error(r);
	curl_easy_cleanup(curl);
        iniparser_free(ini);
	return NULL;
#undef curl_err
}

void
wsmc_handler( WsManClient *cl,
		WsXmlDocH rqstDoc,
		void* user_data)
{
#define curl_err(str)  debug("Error = %d (%s); %s", \
		r, curl_easy_strerror(r), str);
	WsManConnection *con = cl->connection;
	CURL *curl = NULL;
	CURLcode r;
	char *upwd = NULL;
	char *usag = NULL;
	size_t usag_len = 0;
	struct curl_slist *headers=NULL;
	char *buf = NULL;
	int len;
	char *soapact_header = NULL;
	long http_code;
	long auth_avail = 0;
	char *_user = NULL, *_pass = NULL;
	int _no_auth = 0; /* 0 if authentication is used, 1 if no authentication was used */
	u_buf_t *response = NULL;
	//char *soapaction;
	char *tmp_str = NULL;

	if (!cl->initialized && wsmc_transport_init(cl, NULL)) {
		cl->last_error = WS_LASTERR_FAILED_INIT;
		return;
	}
	if (cl->transport == NULL) {
		cl->transport = init_curl_transport(cl);
                if (cl->transport == NULL) {
                        return;
                }
	}
	curl = (CURL *)cl->transport;

	r = curl_easy_setopt(curl, CURLOPT_URL, cl->data.endpoint);
	if (r != CURLE_OK) {
		cl->fault_string = u_strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_URL, ...)");
		goto DONE;
	}

	r = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_handler);
	if (r != CURLE_OK) {
		cl->fault_string = u_strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ..)");
		goto DONE;
	}
	u_buf_create(&response);
	r = curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
	if (r != CURLE_OK) {
		cl->fault_string = u_strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_WRITEDATA, ..)");
		goto DONE;
	}
	char content_type[64];
	snprintf(content_type, 64, "Content-Type: application/soap+xml;charset=%s", cl->content_encoding);
	headers = curl_slist_append(headers, content_type);
	tmp_str = wsman_transport_get_agent(cl);
	usag_len = strlen("User-Agent: ") + strlen(tmp_str) + 1;
	usag = u_malloc(usag_len);
	if (usag == NULL) {
		r = CURLE_OUT_OF_MEMORY;
		cl->fault_string = u_strdup("Could not malloc memory");
		curl_err("Could not malloc memory");
		goto DONE;
	}

	snprintf(usag, usag_len, "User-Agent: %s", tmp_str);
	free(tmp_str);
	headers = curl_slist_append(headers, usag);

#if 0
	soapaction = ws_xml_get_xpath_value(rqstDoc, "/s:Envelope/s:Header/wsa:Action");
	if (soapaction) {
		soapact_header = u_malloc(12 + strlen(soapaction) + 1);
		if (soapact_header) {
			sprintf(soapact_header, "SOAPAction: %s", soapaction);
			headers = curl_slist_append(headers, soapact_header);
		}
		u_free(soapaction);
	}
#endif

	if (cl->flags & WSMAN_CLIENT_SUPRESS_100_CONTINUE) {
		/* Don't request 100-continue */
		headers = curl_slist_append(headers, "Expect:");
	}

	r = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	if (r != CURLE_OK) {
		cl->fault_string = u_strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_HTTPHEADER, ..)");
		goto DONE;
	}

	ws_xml_dump_memory_enc(rqstDoc, &buf, &len, cl->content_encoding);
#if 0
	int count = 0;
	while(count < len) {
		printf("%c",buf[count++]);
	}
#endif
	debug("*****set post buf len = %d******",len);
	r = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
	if (r != CURLE_OK) {
		cl->fault_string = u_strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ..)");
		goto DONE;
	}
	r = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
	if (r != CURLE_OK) {
		cl->fault_string = u_strdup(curl_easy_strerror(r));
		curl_err("Could not curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, ..)");
		goto DONE;
	}

	int iDone = 0;
	while (1) {
		u_free(_user);
		u_free(_pass);
		_user = wsmc_get_user(cl);
		_pass = wsmc_get_password(cl);
		if (_user && _pass && cl->data.auth_set) {
			_no_auth = 0;
			r = curl_easy_setopt(curl, CURLOPT_HTTPAUTH, cl->data.auth_set);
			if (r != CURLE_OK) {
				cl->fault_string = u_strdup(curl_easy_strerror(r));
				curl_err("curl_easy_setopt(CURLOPT_HTTPAUTH) failed");
				goto DONE;
			}
			u_free(upwd);
			upwd = u_strdup_printf(  "%s:%s", _user ,  _pass);
			if (!upwd) {
				r = CURLE_OUT_OF_MEMORY;
				cl->fault_string = u_strdup("Could not malloc memory");
				curl_err("Could not malloc memory");
				goto DONE;
			}
			r = curl_easy_setopt(curl, CURLOPT_USERPWD, upwd);
			if (r != CURLE_OK) {
				cl->fault_string = u_strdup(curl_easy_strerror(r));
				curl_err("curl_easy_setopt(curl, CURLOPT_USERPWD, ..) failed");
				goto DONE;
			}
        } else {
            /* request without user credentials, remember this for
             * later use when it might become necessary to print an error message
             */
            _no_auth = 1;
		}

		if (wsman_debug_level_debugged(DEBUG_LEVEL_MESSAGE)) {
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		}

		r = curl_easy_perform(curl);
		if (r != CURLE_OK) {
			cl->fault_string = u_strdup(curl_easy_strerror(r));
			curl_err("curl_easy_perform failed");
			goto DONE;
		}

		r = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		if (r != CURLE_OK) {
			cl->fault_string = u_strdup(curl_easy_strerror(r));
			curl_err("curl_easy_getinfo(CURLINFO_RESPONSE_CODE) failed");
			goto DONE;
		}

		switch (http_code) 
		{
			case 200:
			case 400:
			case 500:
				// The resource was successfully retrieved or WSMan server 
				// returned a HTTP status code. You can use WinHttpReadData to 
				// read the contents of the server's response.
				iDone = 1;
				break;
			case 401:
				// The server requires authentication.
                /* RFC 2616 states:
                 *
                 * If the request already included Authorization credentials, then the 401
                 * response indicates that authorization has been refused for those
                 * credentials. If the 401 response contains the same challenge as the
                 * prior response, and the user agent has already attempted
                 * authentication at least once, then the user SHOULD be presented the
                 * entity that was given in the response, since that entity might
                 * include relevant diagnostic information.
                 */
                if (_no_auth == 0) {
                    /* no authentication credentials were used. It is only
                     * possible to write a message about the current situation. There
                     * is no information about the last attempt to access the resource.
                     * Maybe at a later point in time I will implement more state information.
                     */
                    fprintf(stdout,"Authentication failed, please retry\n");
                }
				break;
			default:
				// The status code does not indicate success.
				r = CURLE_FAILED_INIT;
				iDone = 1;
				break;
		}

		if(iDone == 1) {
			break;
		}

		/* we are here because of authentication required */
		r = curl_easy_getinfo(curl, CURLINFO_HTTPAUTH_AVAIL, &auth_avail);
		if (r != CURLE_OK) {
			cl->fault_string = u_strdup(curl_easy_strerror(r));
			curl_err("curl_easy_getinfo(CURLINFO_HTTPAUTH_AVAIL) failed");
			goto DONE;
		}

		cl->data.auth_set = reauthenticate(cl, cl->data.auth_set, auth_avail,
                        &cl->data.user, &cl->data.pwd);
                u_buf_clear(response);
                if (cl->data.auth_set == 0) {
                    /* FIXME: user wants to cancel authentication */
#if LIBCURL_VERSION_NUM >= 0x70D01
                    r = CURLE_LOGIN_DENIED;
#else
					/* Map the login failure error to CURLE_LOGIN_DENIED (67) so that we
					 * get the same error code in case of login failure
					 */
					r = 67;
#endif
                    cl->fault_string = u_strdup(curl_easy_strerror(r));
                    curl_err("user/password wrong or empty.");
		    break;
                }
        }
#if 0
	unsigned char *mbbuf = NULL;
	iconv_t cd;
	if(strcmp(cl->content_encoding, "UTF-8")) {
                cd = iconv_open("UTF-8", cl->content_encoding);
                if(cd == -1) {
			cl->last_error = WS_LASTERR_BAD_CONTENT_ENCODING;
			goto DONE2;
                }
                mbbuf = u_zalloc(u_buf_len(response));
                size_t outbuf_len = u_buf_len(response);
                size_t inbuf_len = outbuf_len;
                char *inbuf = u_buf_ptr(response);
                char *outbuf = mbbuf;
                size_t converted = iconv(cd, &inbuf, &inbuf_len, &outbuf, &outbuf_len);
		  iconv_close(cd);
                if( converted == -1) {
			cl->last_error = WS_LASTERR_BAD_CONTENT_ENCODING;
			goto DONE2;
                }
                u_buf_append(con->response, mbbuf, u_buf_len(response) - inbuf_len);
        }
	u_free(mbbuf);
#endif
	u_buf_append(con->response, u_buf_ptr(response), u_buf_len(response));
DONE:
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	cl->response_code = http_code;
	cl->last_error = convert_to_last_error(r);

	debug("curl error code: %d.", r);
	debug("cl->response_code: %d.", cl->response_code);
	debug("cl->last_error code: %d.", cl->last_error);

	curl_slist_free_all(headers);
	u_buf_free(response);
	u_free(soapact_header);
	u_free(usag);
	u_free(upwd);
	u_free(_pass);
	u_free(_user);
#ifdef _WIN32
	ws_xml_free_memory(buf);
#else
	u_free(buf);
#endif

	return;
#undef curl_err

}


int wsmc_transport_init(WsManClient *cl, void *arg)
{
	CURLcode r;

	if (pthread_mutex_lock(&curl_mutex)) {
		error("Error: Can't lock curl_mutex\n");
		return 1;
	}
	if (cl->initialized) {
		if (pthread_mutex_unlock(&curl_mutex)) {
			error("Error: Can't unlock curl_mutex\n");
		}
		return 0;
	}
	r = curl_global_init(CURL_GLOBAL_SSL | CURL_GLOBAL_WIN32);
	if (r == CURLE_OK) {
		cl->initialized = 1;
	}
	if (pthread_mutex_unlock(&curl_mutex)) {
		error("Error: Can't unlock curl_mutex\n");
	}
	if (r != CURLE_OK) {
		debug("Error = %d (%s); Could not initialize curl globals",
				r, curl_easy_strerror(r));
	}
	return (r == CURLE_OK ? 0 : 1);
}

void wsmc_transport_fini(WsManClient *cl)
{
	if (pthread_mutex_lock(&curl_mutex)) {
		error("Error: Can't lock curl_mutex\n");
		return;
	}
	if (cl->initialized == 0 ) {
		if (pthread_mutex_unlock(&curl_mutex)) {
			error("Error: Can't unlock curl_mutex\n");
		}
		return;
	}
	curl_global_cleanup();
	cl->initialized = 0;
	if (pthread_mutex_unlock(&curl_mutex)) {
		error("Error: Can't unlock curl_mutex\n");
	}
	return;
}

void
wsman_transport_close_transport(WsManClient *cl)
{
	if (cl->transport != NULL) {
		curl_easy_cleanup((CURL *)cl->transport);
	}
	cl->transport = NULL;
}


