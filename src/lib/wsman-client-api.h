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
 * @author Sumeet Kukreja, Dell Inc.
 */

#ifndef WSMANCLIENT_API_H_
#define WSMANCLIENT_API_H_


#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-names.h"
#include "wsman-types.h"


// Possible authentication methods

typedef enum {
    WS_NO_AUTH,
    WS_BASIC_AUTH,
    WS_DIGEST_AUTH,
    WS_PASS_AUTH,
    WS_NTLM_AUTH,
    WS_GSSNEGOTIATE_AUTH,
    WS_MAX_AUTH,
} wsman_auth_type_t;

        typedef void (*wsman_auth_request_func_t)( wsman_auth_type_t t, 
                    char **usr, 
                    char **pwd);

	struct _WsManClient;
	typedef struct _WsManClient WsManClient;

	typedef enum {
		WS_LASTERR_OK = 0,
		WS_LASTERR_OTHER_ERROR,	// not recognized error
		WS_LASTERR_FAILED_INIT,
		WS_LASTERR_UNSUPPORTED_PROTOCOL,
		WS_LASTERR_URL_MALFORMAT,
		WS_LASTERR_COULDNT_RESOLVE_PROXY,
		WS_LASTERR_COULDNT_RESOLVE_HOST,
		WS_LASTERR_COULDNT_CONNECT,
		WS_LASTERR_HTTP_RETURNED_ERROR,
		WS_LASTERR_WRITE_ERROR,
		WS_LASTERR_READ_ERROR,
		WS_LASTERR_OUT_OF_MEMORY,
		WS_LASTERR_OPERATION_TIMEOUTED,	// the timeout time was reached
		WS_LASTERR_HTTP_POST_ERROR,
		WS_LASTERR_BAD_DOWNLOAD_RESUME,	//couldn't resume download
		WS_LASTERR_TOO_MANY_REDIRECTS,	// catch endless re-direct loops
		WS_LASTERR_SSL_CONNECT_ERROR,
		WS_LASTERR_SSL_PEER_CERTIFICATE,	// peer's certificate wasn't ok
		WS_LASTERR_SSL_ENGINE_NOTFOUND,	// SSL crypto engine not found
		WS_LASTERR_SSL_ENGINE_SETFAILED,	// can't set SSL crypto engine default
		WS_LASTERR_SSL_CERTPROBLEM,	// problem with the local certificate
		WS_LASTERR_SSL_CACERT,	// problem with the CA cert (path?)
		WS_LASTERR_SSL_ENGINE_INITFAILED,	// failed to initialise ENGINE
		WS_LASTERR_SEND_ERROR,	// failed sending network data
		WS_LASTERR_RECV_ERROR,	// failure in receiving network data
		WS_LASTERR_BAD_CONTENT_ENCODING,	// Unrecognized transfer encoding
		WS_LASTERR_LOGIN_DENIED,	// user, password or similar was not
		// accepted and we failed to login

		WS_LASTERR_LAST	// never use!
	} WS_LASTERR_Code;


	typedef enum {
		WSMAN_ACTION_NONE = 0,
		WSMAN_ACTION_TRANSFER_GET,
		WSMAN_ACTION_TRANSFER_PUT,
		WSMAN_ACTION_ENUMERATION,
		WSMAN_ACTION_PULL,
		WSMAN_ACTION_RELEASE,
		WSMAN_ACTION_CUSTOM,
		WSMAN_ACTION_TRANSFER_CREATE,
		WSMAN_ACTION_TRANSFER_DELETE,
		WSMAN_ACTION_IDENTIFY,
		WSMAN_ACTION_TEST
	} WsmanAction;



// options flags values
#define FLAG_NONE                            0x0000
#define FLAG_ENUMERATION_COUNT_ESTIMATION    0x0001
#define FLAG_ENUMERATION_OPTIMIZATION        0x0002
#define FLAG_ENUMERATION_ENUM_EPR            0x0004
#define FLAG_ENUMERATION_ENUM_OBJ_AND_EPR    0x0008
#define FLAG_DUMP_REQUEST                    0x0010
#define FLAG_IncludeSubClassProperties       0x0020
#define FLAG_ExcludeSubClassProperties       0x0040
#define FLAG_POLYMORPHISM_NONE               0x0080
#define FLAG_MUND_MAX_ESIZE                  0x0100
#define FLAG_MUND_LOCALE                     0x0200
#define FLAG_MUND_OPTIONSET                  0x0400
#define FLAG_MUND_FRAGMENT                   0x0800
#define FLAG_CIM_EXTENSIONS                  0x1000
#define FLAG_CIM_REFERENCES                  0x2000
#define FLAG_CIM_ASSOCIATORS                 0x4000

	typedef struct {
		unsigned long flags;
		char *filter;
		char *dialect;
		char *fragment;
		char *cim_ns;
		hash_t *selectors;
		hash_t *properties;
		unsigned int timeout;
		unsigned int max_envelope_size;
		unsigned int max_elements;

	} client_opt_t;



	struct _WsManFault {
		const char *code;
		const char *subcode;
		const char *reason;
		const char *fault_detail;
	};
	typedef struct _WsManFault WsManFault;



	/**
	 * Create a client using an endpoint as the argument
	 * @param endpoint an URI describing the endpoint, with user/pass, port and path
	 * @return client handle
	 */
	WsManClient *wsmc_create_from_uri(const char *endpoint);


	/**
 	* Create client and initialize context
 	* @param hostname Hostname or IP
 	* @param port port
 	* @param path HTTP path, for example /wsman
 	* @param scheme scheme, HTTP or HTTPS
 	* @param username User name
 	* @param password Passwrod
 	* @return client handle
 	*/
	WsManClient *wsmc_create(const char *hostname,
					 const int port, const char *path,
					 const char *scheme,
					 const char *username,
					 const char *password);

	/*
	 * Release client
	 * @param cl Client handle that was created with wsman_create_client
	 * @return void
	 */
	void wsmc_release(WsManClient * cl);

	/*
	 * Reset client connection and prepare handle for a new connection
	 * @param cl Client handle
	 * @return void
	 */
	void wsmc_reinit_conn(WsManClient * cl);


	/* WsManClient handling */
	WsContextH wsmc_get_context(WsManClient * cl);

	char *wsmc_get_hostname(WsManClient * cl);

	unsigned int wsmc_get_port(WsManClient * cl);

	char *wsmc_get_path(WsManClient * cl);

	char *wsmc_get_scheme(WsManClient * cl);

	char *wsmc_get_user(WsManClient * cl);

	char *wsmc_get_endpoint(WsManClient * cl);

	long wsmc_get_response_code(WsManClient * cl);

	char *wsmc_get_fault_string(WsManClient * cl);

	WS_LASTERR_Code wsmc_get_last_error(WsManClient * cl);


	/**
	 * Read XML file
	 * @param cl Client handle
	 * @param filename File name
	 * @param encoding Encoding
	 * @param XML options
	 * @return XML document
	 */
	WsXmlDocH wsmc_read_file(WsManClient * cl, char *filename,
					 char *encoding,
					 unsigned long options);
	/**
	 * Read buffer into an XML document
	 * @param cl Client handle
	 * @param buf Buffer with xml text
	 * @param size Size of buffer
	 * @param encoding Encoding
	 * @param XML options
	 * @return XML document
	 */
	WsXmlDocH wsmc_read_memory(WsManClient * cl, char *buf,
					   size_t size, char *encoding,
					   unsigned long options);

	/* Creating objects */
	WsContextH wsman_create_runtime(void);

	WsXmlDocH wsman_build_envelope(WsContextH cntx, const char *action,
				       const char *reply_to_uri,
				       const char *resource_uri,
				       const char *to_uri,
				       client_opt_t * options);

	WsXmlDocH wsman_build_envelope_from_response(WsManClient * cl);

	WsXmlDocH wsmc_build_envelope_from_response(WsManClient * cl);

	/* Wsman actions handling */

	WsXmlDocH wsmc_action_identify(WsManClient * cl,
				 client_opt_t * options);

	WsXmlDocH wsmc_action_get(WsManClient * cl,
				  const char *resourceUri,
				  client_opt_t * options);

	WsXmlDocH wsmc_action_put(WsManClient * cl,
				  const char *resource_uri,
				  client_opt_t * options,
				  WsXmlDocH source_doc);

	WsXmlDocH wsmc_action_put_fromtext(WsManClient * cl,
					   const char *resource_uri,
					   client_opt_t * options,
					   const char *data, size_t size,
					   const char *encoding);

	WsXmlDocH wsmc_action_put_serialized(WsManClient * cl,
					     const char *resource_uri,
					     client_opt_t * options,
					     void *typeInfo, void *data);

	WsXmlDocH wsmc_action_get_and_put(WsManClient * cl,
					  const char *resourceUri,
					  client_opt_t * options);

	WsXmlDocH wsmc_action_create(WsManClient * cl,
				     const char *resourceUri,
				     client_opt_t * options,
				     WsXmlDocH source_doc);

	WsXmlDocH wsmc_action_create_fromtext(WsManClient * cl,
					      const char *resourceUri,
					      client_opt_t * options,
					      const char *data,
					      size_t size,
					      const char *encoding);

	WsXmlDocH wsmc_action_create_serialized(WsManClient * cl,
						const char *resource_uri,
						client_opt_t * options,
						void *typeInfo,
						void *data);

	WsXmlDocH wsmc_action_delete(WsManClient * cl,
				     const char *resourceUri,
				     client_opt_t * options);

	WsXmlDocH wsmc_action_enumerate(WsManClient * cl,
				   const char *resourceUri,
				   client_opt_t * options);

	WsXmlDocH wsmc_action_pull(WsManClient * cl, const char *resourceUri,
			      client_opt_t * options,
			      const char *enumContext);

	WsXmlDocH wsmc_action_release(WsManClient * cl, const char *resourceUri,
				 client_opt_t * options,
				 const char *enumContext);

	WsXmlDocH wsmc_action_invoke(WsManClient * cl, const char *resourceUri,
			       client_opt_t * options, const char *method,
			       WsXmlDocH source_doc);

	WsXmlDocH wsmc_action_invoke_fromtext(WsManClient * cl,
					const char *resourceUri,
					client_opt_t * options,
					const char *method,
					const char *data, size_t size,
					const char *encoding);

	WsXmlDocH wsmc_action_invoke_serialized(WsManClient * cl,
					  const char *resourceUri,
					  client_opt_t * options,
					  const char *method,
					  void *typeInfo, void *data);

	typedef int (*SoapResponseCallback) (WsManClient *, WsXmlDocH,
					     void *);

	int wsmc_action_enumerate_and_pull(WsManClient * cl,
				      const char *resource_uri,
				      client_opt_t * options,
				      SoapResponseCallback callback,
				      void *callback_data);

	WsXmlDocH wsmc_create_request(WsManClient * cl,
					      const char *resource_uri,
					      client_opt_t * options,
					      WsmanAction action,
					      char *method, void *data);

	/* WsXmlDocH manipulation */
	char *wsmc_get_enum_context(WsXmlDocH doc);

	void wsmc_free_enum_context(char *);

	void wsmc_add_fragment_transfer(WsXmlDocH doc, char *fragment);

	void wsmc_add_namespace_as_selector(WsXmlDocH doc, char *ns);

	void wsmc_add_selector_from_uri(WsXmlDocH doc,
					 const char *resourceUri);


	void wsman_build_assocRef_body(WsManClient *cl, WsXmlNodeH body,
		       	const char *resource_uri,
			client_opt_t *options, int assocRef);

	/* Action options handling */
	client_opt_t *wsmc_options_init(void);

	void wsmc_options_destroy(client_opt_t * op);

	void wsmc_add_selectors_from_str(client_opt_t * options,
						   const char
						   *query_string);

	void wsmc_add_prop_from_str(client_opt_t * options,
						    const char
						    *query_string);

	void wsmc_add_selector_from_options(WsXmlDocH doc,
					     client_opt_t * options);

	void wsmc_set_action_option(client_opt_t * options,
				     unsigned int);

	void wsmc_set_options_from_uri(const char *resourceUri,
					client_opt_t * options);

	void wsmc_add_selector(client_opt_t * options,
				       const char *key, const char *value);

	void wsmc_add_property(client_opt_t * options,
				       const char *key, const char *value);

	/* Misc */

	/* Place holder */
	void wsman_remove_query_string(const char *resourceUri,
				       char **result);
	int wsmc_check_for_fault(WsXmlDocH doc);

	void wsmc_node_to_buf(WsXmlNodeH node, char **buf);

	char *wsmc_node_to_formatbuf(WsXmlNodeH node);

	void wsmc_get_fault_data(WsXmlDocH doc,
					 WsManFault * fault);

	WsManFault *wsmc_fault_new(void);

	void wsmc_set_dumpfile(WsManClient *cl, FILE * f);

	FILE *wsmc_get_dumpfile(WsManClient *cl);

#if 0

	int wsman_session_open(const char *server,
			       int port,
			       const char *path,
			       const char *scheme,
			       const char *username,
			       const char *password, int flags);

	char wsman_session_close(int session_id);

	const char *wsman_session_error(int session_id);

	char wsman_session_set_server(int sid,
				      const char *server,
				      int port,
				      const char *path,
				      const char *scheme,
				      const char *username,
				      const char *password);

	char wsman_session_resource_locator_set(int session_id,
						const char *resource_uri);

	int wsman_session_resource_locator_new(int session_id,
					       const char *resource_uri);
	char wsman_session_resource_locator_add_selector(int session_id,
							 const char *name,
							 const char
							 *value);
	char wsman_session_resource_locator_clear_selectors(int
							    session_id);

	char *wsman_session_identify(int session_id, int flag);

	int wsman_session_enumerate(int session_id,
				    const char *resource_uri,
				    const char *filter,
				    const char *dialect, int flags);

	char wsman_enumerator_release(int enumerator_id);
	char wsman_enumerator_end(int enumerator_id);
	char *wsman_enumerator_pull(int enumerator_id);
	const char *wsman_enumerator_error(int enumerator_id);

	char *wsman_session_transfer_get(int session_id, int flags);
	char *wsman_session_transfer_put(int session_id,
					 const char *xml_content,
					 int flags);
	char *wsman_session_transfer_create(int session_id,
					    const char *xml_content,
					    int flag);

	char *wsman_session_invoke(int sid,
				   const char *method,
				   const char *xml_content, int flag);

	char *wsman_session_serialize(int sid, void *data,
				      void *type_info);
#endif

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* WSMANCLIENT_H_ */
