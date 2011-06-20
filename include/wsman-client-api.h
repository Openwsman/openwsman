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
 */

#ifndef WSMANCLIENT_API_H_
#define WSMANCLIENT_API_H_


#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#include "wsman-xml-api.h"
#include "wsman-names.h"
#include "wsman-types.h"
#include "wsman-xml-serializer.h"
#include "wsman-epr.h"
#include "wsman-filter.h"

/**
 * @defgroup Client Client
 * @brief WS-Management Client
 *
 * @{
 */


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

	struct _WsManClient;
	typedef struct _WsManClient WsManClient;

        typedef void (*wsman_auth_request_func_t)( WsManClient *client, wsman_auth_type_t t,
                    char **usr,
                    char **pwd);

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

		WS_LASTERR_BAD_CRL_FILE, //bad CRL file provided (Format, Path or permission)
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
		WSMAN_ACTION_ANON_IDENTIFY,
		WSMAN_ACTION_SUBSCRIBE,
		WSMAN_ACTION_UNSUBSCRIBE,
		WSMAN_ACTION_RENEW,
		WSMAN_ACTION_ASSOCIATORS,
		WSMAN_ACTION_REFERENCES,
		WSMAN_ACTION_TEST
	} WsmanAction;

typedef enum {
	WSMAN_DELIVERY_PUSH = 0,
	WSMAN_DELIVERY_PUSHWITHACK,
	WSMAN_DELIVERY_EVENTS,
	WSMAN_DELIVERY_PULL
}WsmanDeliveryMode;

typedef enum {
	WSMAN_DELIVERY_SEC_AUTO = 0,
	WSMAN_DELIVERY_SEC_HTTP_BASIC,
	WSMAN_DELIVERY_SEC_HTTP_DIGEST,
	WSMAN_DELIVERY_SEC_HTTPS_BASIC,
	WSMAN_DELIVERY_SEC_HTTPS_DIGEST,
	WSMAN_DELIVERY_SEC_HTTPS_MUTUAL,
	WSMAN_DELIVERY_SEC_HTTPS_MUTUAL_BASIC,
	WSMAN_DELIVERY_SEC_HTTPS_MUTUAL_DIGEST,
	WSMAN_DELIVERY_SEC_HTTPS_SPNEGO_KERBEROS,
	WSMAN_DELIVERY_SEC_HTTPS_MUTUAL_SPNEGO_KERBEROS,
	WSMAN_DELIVERY_SEC_HTTP_SPNEGO_KERBEROS
}WsManDeliverySecurityMode;

// options flags values
#define FLAG_NONE                            0x0000
#define FLAG_ENUMERATION_COUNT_ESTIMATION    0x0001
#define FLAG_ENUMERATION_OPTIMIZATION        0x0002
#define FLAG_ENUMERATION_ENUM_EPR            0x0004
#define FLAG_ENUMERATION_ENUM_OBJ_AND_EPR    0x0008
#define FLAG_DUMP_REQUEST                    0x0010
#define FLAG_IncludeSubClassProperties       0x0020
#define FLAG_INCLUDESUBCLASSPROPERTIES       FLAG_IncludeSubClassProperties
#define FLAG_ExcludeSubClassProperties       0x0040
#define FLAG_EXCLUDESUBCLASSPROPERTIES       FLAG_ExcludeSubClassProperties
#define FLAG_POLYMORPHISM_NONE               0x0080
#define FLAG_MUND_MAX_ESIZE                  0x0100
#define FLAG_MUND_LOCALE                     0x0200
#define FLAG_MUND_OPTIONSET                  0x0400
#define FLAG_MUND_FRAGMENT                   0x0800
#define FLAG_CIM_EXTENSIONS                  0x1000
#define FLAG_CIM_REFERENCES                  0x2000
#define FLAG_CIM_ASSOCIATORS                 0x4000
#define FLAG_EVENT_SENDBOOKMARK		     0X8000
#define FLAG_CIM_SCHEMA_OPT		    0X10000
#define FLAG_EXCLUDE_NIL_PROPS		    0X20000

	typedef struct {
		unsigned long flags;
		char *fragment;
		char *cim_ns;
		char * delivery_uri;
		char * reference;
		WsmanDeliveryMode delivery_mode; //eventing delivery mode
		WsManDeliverySecurityMode delivery_sec_mode; //security mode of eventing delivery
		char *delivery_username; // username for delivery, if it is necessary
		char *delivery_password; // password for delivery, if it is necessary
		char *delivery_certificatethumbprint; // certificate thumbprint of event sink, if it is necessary
		float heartbeat_interval;
		float expires;
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

	/**
	* Set request/response content encoding type. Default encoding type is "UTF-8"
	* @param cl Client handle
	* @param encoding type of encoding, for example "UTF-16"
	* @ return zero for success, others for an error
	*/
	int wsmc_set_encoding(WsManClient *cl, const char *encoding);

	/**
	* Set request CIM namespace
	* @param cl Client handle
	* @param ns requested CIM class namespace
	* @ return zero for success, others for an error
	*/
	int wsmc_set_namespace(WsManClient *cl, const char *ns);

	/**
        * Return request CIM namespace
        * @param cl Client handle
        * @ return requested CIM class namespace
        */
	char *wsmc_get_namespace(WsManClient *cl);

	/**
	 * Release client
	 * @param cl Client handle that was created with wsman_create_client
	 * @return void
	 */
	void wsmc_release(WsManClient * cl);

	/**
	 * Reset client connection and prepare handle for a new connection
	 * @param cl Client handle
	 * @return void
	 */
	void wsmc_reinit_conn(WsManClient * cl);


	/* WsManClient handling */

	/**
	 * Get serialization context
	 * @param cl Client handle
	 * @return Context
	 */
	WsSerializerContextH wsmc_get_serialization_context(WsManClient * cl);

	/**
	 * Get host name from handle
	 * @param cl Client handle
	 * @return host name
	 */
	char *wsmc_get_hostname(WsManClient * cl);

	/**
	 * Get port from handle
	 * @param cl Client handle
	 * @return host name
	 */
	unsigned int wsmc_get_port(WsManClient * cl);

	/**
	 * Get uri path from handle
	 * @param cl Client handle
	 * @return uri path
	 */
	char *wsmc_get_path(WsManClient * cl);

	/**
	 * Get uri scheme from handle
	 * @param cl Client handle
	 * @return scheme
	 */
	char *wsmc_get_scheme(WsManClient * cl);

	/**
	 * Get username from handle
	 * @param cl Client handle
	 * @return username
	 */
	char *wsmc_get_user(WsManClient * cl);

	/**
	 * Get password from handle
	 * @param cl Client handle
	 * @return username
	 */
	char *wsmc_get_password(WsManClient * cl);

	/**
	* Get request/response content encoding type. Default encoding type is "UTF-8"
	* @param cl Client handle
	* @ return request encoding string
	*/
	char *wsmc_get_encoding(WsManClient *cl);

	/**
	 * Get endpoint from handle
	 * @param cl Client handle
	 * @return endpoint
	 */
	char *wsmc_get_endpoint(WsManClient * cl);

	/**
	 * Get response code from client
	 * @param cl Client handle
	 * @return response code
	 */
	long wsmc_get_response_code(WsManClient * cl);

	/**
	 * Get fault string
	 * @param cl Client handle
	 * @return host name
	 */
	char *wsmc_get_fault_string(WsManClient * cl);

	/**
	 * Get last error code
	 * @param cl Client handle
	 * @return last error code
	 */
	WS_LASTERR_Code wsmc_get_last_error(WsManClient * cl);


	/**
	 * Read XML file
	 * @param cl Client handle
	 * @param filename File name
	 * @param encoding Encoding
	 * @param XML options
	 * @return XML document
	 */
	WsXmlDocH wsmc_read_file( const char *filename,
					 const char *encoding,
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
	WsXmlDocH wsmc_read_memory( char *buf,
					   size_t size, const char *encoding,
					   unsigned long options);

	/**
	 * Create runtime context for client
	 * @param cl Client handle
	 * @return void
	 */
	WsContextH wsmc_create_runtime(void);

	/**
	 * Parse response and create a new envelope based on it
	 * @param cl Client handle
	 * @return New envelope
	 */
	WsXmlDocH wsmc_build_envelope_from_response(WsManClient * cl);

	/* Wsman actions handling */

	/**
	 * Send an Identify request
	 * @param cl Client handle
	 * @param client_opt_t Request options and flags
	 * @return response document
	 */
	WsXmlDocH wsmc_action_identify(WsManClient * cl,
				 client_opt_t * options);


	/**
	 * Send a Transfer Get request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @return response document
	 */
	WsXmlDocH wsmc_action_get(WsManClient * cl,
				  const char *resource_uri,
				  client_opt_t * options);


	WsXmlDocH wsmc_action_get_from_epr(WsManClient *cl, epr_t *epr,
			        client_opt_t *options);



	/**
	 * Send a Transfer Put request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param source_doc A document with the new resource, for example the result of a Get request with modified properties.
	 * @return response document
	 */
	WsXmlDocH wsmc_action_put(WsManClient * cl, const char *resource_uri, client_opt_t * options,
				  WsXmlDocH source_doc);

	/**
	 * Send a Transfer Put request using a text buffer XML representation
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param data Buffer
	 * @param size Buffer size
	 * @param encoding XML encoding
	 * @return response document
	 */
	WsXmlDocH wsmc_action_put_fromtext(WsManClient * cl,
					   const char *resource_uri,
					   client_opt_t * options,
					   const char *data, size_t size,
					   const char *encoding);

	/**
	 * Send a Transfer Put request using serialized data
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param typeInfo Data type information
	 * @param data  Pointer to data
	 * @return response document
	 */
	WsXmlDocH wsmc_action_put_serialized(WsManClient * cl,
					     const char *resource_uri,
					     client_opt_t * options,
					     void *typeInfo, void *data);

	/**
	 * Send a Transfer Get and then Transfer Put modifying properties defined in the options structure
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @return response document
	 */
	WsXmlDocH wsmc_action_get_and_put(WsManClient * cl,
					  const char *resource_uri,
					  client_opt_t * options);

	/**
	 * Send a Transfer Create using existing document with a new resource
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param source_doc A document with the new resource, for example the result of a
     *        Get request with modified properties.
	 * @return response document
	 */
	WsXmlDocH wsmc_action_create(WsManClient * cl, const char *resource_uri, client_opt_t * options,
				     WsXmlDocH source_doc);

	/**
	 * Send a Transfer Create request using a text buffer XML representation
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param data Buffer
	 * @param size Buffer size
	 * @param encoding XML encoding
	 * @return response document
	 */
	WsXmlDocH wsmc_action_create_fromtext(WsManClient * cl, const char *resource_uri, client_opt_t * options,
					      const char *data, size_t size, const char *encoding);

	/**
	 * Send a Transfer Create request using serialized data
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param typeInfo Data type information
	 * @param data  Pointer to data
	 * @return response document
	 */
	WsXmlDocH wsmc_action_create_serialized(WsManClient * cl,
						const char *resource_uri,
						client_opt_t * options,
						void *typeInfo,
						void *data);

	/**
	 * Send a Transfer Delete
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @return response document
	 */
	WsXmlDocH wsmc_action_delete(WsManClient * cl,
				     const char *resource_uri,
				     client_opt_t * options);

	WsXmlDocH wsmc_action_delete_from_epr(WsManClient *cl, epr_t *epr,
			        client_opt_t *options);

	/**
	 * Send a Enumerate request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @return response document
	 */
	WsXmlDocH wsmc_action_enumerate(WsManClient * cl,
				   const char *resource_uri,
				   client_opt_t * options,
				   filter_t *filter
				   );

	/**
	 * Send a Pull request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param enumContext enumeration context
	 * @return response document
	 */
	WsXmlDocH wsmc_action_pull(WsManClient * cl, const char *resource_uri,
			      client_opt_t * options,
				   filter_t *filter,
			      const char *enumContext);

	/**
	 * Send a Release request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param enumContext enumeration context
	 * @return response document
	 */
	WsXmlDocH wsmc_action_release(WsManClient * cl, const char *resource_uri,
				 client_opt_t * options,
				 const char *enumContext);

	/**
	 * Send a Release request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @return response document
	 */
	WsXmlDocH wsmc_action_subscribe(WsManClient * cl, const char *resource_uri,
				 client_opt_t * options, filter_t *filter);


	/**
	 * Send a Release request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param uuid Subscription reference parameter
	 * @return response document
	 */
	WsXmlDocH wsmc_action_unsubscribe(WsManClient * cl, const char *resource_uri,
				 client_opt_t * options,
				 const char *subsContext);


	/**
	 * Send a Release request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param uuid Subscription reference parameter
	 * @return response document
	 */
	WsXmlDocH wsmc_action_renew(WsManClient * cl, const char *resource_uri,
				 client_opt_t * options,
				 const char *subsContext);



	/**
	 * Send a custom method request
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param method  Custom method name
	 * @param source_doc A document with the new resource, for example the result of a Get request with modified properties.
	 * @return response document
	 */
	WsXmlDocH wsmc_action_invoke(WsManClient * cl, const char *resource_uri,
			       client_opt_t * options, const char *method,
			       WsXmlDocH source_doc);

	/**
	 * Send a custom method request using text input of XML data
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param method  Custom method name
	 * @param data Buffer
	 * @param size Buffer size
	 * @param encoding XML encoding
	 * @return response document
	 */
	WsXmlDocH wsmc_action_invoke_fromtext(WsManClient * cl,
					const char *resource_uri,
					client_opt_t * options,
					const char *method,
					const char *data, size_t size,
					const char *encoding);

	/**
	 * Send a custom method request using serialized data of resource
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param method  Custom method name
	 * @param typeInfo Data type information
	 * @param data  Pointer to data
	 * @return response document
	 */
	WsXmlDocH wsmc_action_invoke_serialized(WsManClient * cl,
					  const char *resource_uri,
					  client_opt_t * options,
					  const char *method,
					  void *typeInfo, void *data);

	typedef int (*SoapResponseCallback) (WsManClient *, WsXmlDocH,
					     void *);

	/**
	 * Send a Enumerate request and then use callback for subsequent Pull calls
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param callback Function to handle Pull requests and Responses
	 * @param callback_data Pointer to callback data
	 * @return success
	 */
	int wsmc_action_enumerate_and_pull(WsManClient * cl,
				      const char *resource_uri,
				      client_opt_t * options,
					  filter_t *filter,
				      SoapResponseCallback callback,
				      void *callback_data);

	/**
	 * Create a request envelope based on client data
	 * @param cl Client handle
	 * @param resource_uri Resource URI
	 * @param client_opt_t Request options and flags
	 * @param action Requested Action (e.g., Get, Put, Create)
	 * @param method custom method ( can be NULL)
	 * @param data client data
	 * @return Request document
	 */
	WsXmlDocH wsmc_create_request(WsManClient * cl,
					      const char *resource_uri,
					      client_opt_t * options,
					      filter_t *filter,
					      WsmanAction action,
					      char *method, void *data);

	/**
	 * Get enumeration context from response
	 * @param doc Response document
	 * @return enumeration context
	 */
	char *wsmc_get_enum_context(WsXmlDocH doc);

	/**
	* Get enumeration context from subscription response
	* @param doc Response document
	* @return enumeration context
	*/
	char *wsmc_get_event_enum_context(WsXmlDocH doc);

	/**
	 * Free enumeration context
	 * @param enumctx Enumeration context
	 * @return enumeration context
	 */
	void wsmc_free_enum_context(char * enumctx);

	/**
	 * Add a selector from URI with query string containing selectors
	 * @param doc request to add selectors to
	 * @param resource_uri Resource URI
	 * @return void
	 */
	void wsmc_add_selector_from_uri(WsXmlDocH doc,
					 const char *resource_uri);


	/**
	 * Initialize client requeust options
	 * @return initialized options structure
	 */
	client_opt_t *wsmc_options_init(void);

	/**
	 * destroy client request options
	 * @param options structure
	 * @return void
	 */
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

	void wsmc_clear_action_option(client_opt_t * options,
				     unsigned int);

	void wsmc_set_options_from_uri(const char *resource_uri,
					client_opt_t * options);

	void wsmc_set_filter(filter_t *filter, client_opt_t * options);

	void wsmc_add_selector(client_opt_t * options,
				       const char *key, const char *value);

	void wsmc_add_property(client_opt_t * options,
				       const char *key, const char *value);

	/* Misc */

	/* Place holder */
	void wsmc_remove_query_string(const char *resource_uri,
				       char **result);
	int wsmc_check_for_fault(WsXmlDocH doc);

	void wsmc_node_to_buf(WsXmlNodeH node, char **buf);

	char *wsmc_node_to_formatbuf(WsXmlNodeH node);

	void wsmc_get_fault_data(WsXmlDocH doc,
					 WsManFault * fault);

	WsManFault *wsmc_fault_new(void);


	void wsmc_fault_destroy(WsManFault *fault);

	void wsmc_set_dumpfile(WsManClient *cl, FILE * f);

	FILE *wsmc_get_dumpfile(WsManClient *cl);

#ifndef _WIN32
	void wsmc_set_conffile(WsManClient *cl, char * f);

	char * wsmc_get_conffile(WsManClient *cl);
#endif
	void
	wsmc_set_delivery_uri(const char *delivery_uri, client_opt_t * options);


	void
	wsmc_set_sub_expiry(int event_subscription_expire, client_opt_t * options);


	void
	wsmc_set_heartbeat_interval(int heartbeat_interval, client_opt_t * options);


	void
	wsmc_set_delivery_mode(WsmanDeliveryMode delivery_mode, client_opt_t * options);


	void
	wsmc_set_delivery_security_mode(WsManDeliverySecurityMode delivery_sec_mode, client_opt_t * options);


/** @} */


#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* WSMANCLIENT_H_ */
