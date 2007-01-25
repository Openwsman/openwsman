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
extern          "C" {
#endif				/* __cplusplus */

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-names.h"
#include "wsman-types.h"


	struct _WsManClient;
	typedef struct _WsManClient WsManClient;


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
	}               WsmanAction;



	typedef struct {
		unsigned char   flags;
		char           *filter;
		char           *dialect;
		char           *fragment;
		char           *cim_ns;
		hash_t         *selectors;
		hash_t         *properties;
		unsigned int    timeout;
		unsigned int    max_envelope_size;
		unsigned int    max_elements;

	}               actionOptions;


	typedef int     (*SoapResponseCallback) (WsManClient *, WsXmlDocH, void *);




	/**
	 * Create a client using an endpoint as the argument
	 * @param endpoint an URI describing the endpoint, with user/pass, port and path
	 * @return client handle
	 */
	WsManClient    *wsman_create_client_from_uri(const char* endpoint);


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
	WsManClient    *wsman_create_client(const char *hostname,
	                const int port, const char *path, const char *scheme,
		                const char *username, const char *password);

	/*
	 * Release client
	 * @param cl Client handle that was created with wsman_create_client
	 * @return void
	 */
	void            wsman_release_client(WsManClient * cl);
	/*
	 * Reset client connection and prepare handle for a new connection
	 * @param cl Client handle
	 * @return void
	 */
	void            reinit_client_connection(WsManClient * cl);


	/* WsManClient handling */


	long          wsman_get_client_response_code(WsManClient * cl);
    char *        wsman_get_client_hostname(WsManClient * cl);
    unsigned int  wsman_get_client_port(WsManClient * cl);
    char *        wsman_get_client_path(WsManClient * cl);
    char *        wsman_get_client_user(WsManClient * cl);
    char *        wsman_get_client_endpoint(WsManClient * cl);
    long          wsman_get_client_response_code(WsManClient * cl);
    long          wsman_get_client_response_code(WsManClient * cl);


    WsContextH      wsman_client_get_context(WsManClient * cl);
	long            wsman_client_get_response_code(WsManClient * cl);
    char           *wsman_client_get_fault_string(WsManClient * cl);

    char           *wsman_client_get_hostname(WsManClient * cl);
    unsigned int    wsman_client_get_port(WsManClient * cl);
    char           *wsman_client_get_scheme(WsManClient * cl);
    char           *wsman_client_get_path(WsManClient * cl);
    char           *wsman_client_get_user(WsManClient * cl);
    char           *wsman_client_get_endpoint(WsManClient * cl);
    long            wsman_client_get_response_code(WsManClient * cl);





	/**
	 * Read XML file
	 * @param cl Client handle
	 * @param filename File name
	 * @param encoding Encoding
	 * @param XML options
	 * @return XML document
	 */
	WsXmlDocH       wsman_client_read_file(WsManClient * cl, char *filename,
		                     char *encoding, unsigned long options);
	/**
	 * Read buffer into an XML document
	 * @param cl Client handle
	 * @param buf Buffer with xml text
	 * @param size Size of buffer
	 * @param encoding Encoding
	 * @param XML options
	 * @return XML document
	 */
	WsXmlDocH       wsman_client_read_memory(WsManClient * cl, char *buf,
	                   int size, char *encoding, unsigned long options);



            /* Creating objects */

	WsContextH      wsman_create_runtime(void);
	WsXmlDocH       wsman_create_doc(WsContextH cntx, char *rootname);
	WsXmlDocH       wsman_build_envelope(WsContextH cntx, char *action,
		                     char *reply_to_uri, char *resource_uri,
		                       char *to_uri, actionOptions options);
	WsXmlDocH       wsman_build_envelope_from_response(WsManClient * cl);
	WsXmlDocH       wsman_client_build_envelope_from_response(WsManClient * cl);

                /* Wsman actions handling */

	WsXmlDocH       wsman_identify(WsManClient * cl, actionOptions options);
	
	WsXmlDocH       ws_transfer_get(WsManClient * cl, char *resourceUri,
				                     actionOptions options);
	WsXmlDocH       ws_transfer_put(WsManClient * cl, char *resource_uri, void *data,
		                    actionOptions options);
	WsXmlDocH		ws_transfer_put_serialized(WsManClient * cl,
           					char *resource_uri, void *data, void *typeInfo, 
           					actionOptions options);
	WsXmlDocH       ws_transfer_get_and_put(WsManClient * cl, char *resourceUri,
				                     actionOptions options);
	WsXmlDocH       ws_transfer_create(WsManClient * cl, char *resourceUri,
				                 void *data, actionOptions options);
	WsXmlDocH		ws_transfer_create_serialized(WsManClient * cl,
           					char *resource_uri, void *data, void *typeInfo, 
           					actionOptions options);			
	WsXmlDocH       ws_transfer_delete(WsManClient * cl, char *resourceUri,
				                     actionOptions options);
	WsXmlDocH       wsenum_enumerate(WsManClient * cl, char *resourceUri,
				                     actionOptions options);
	WsXmlDocH       wsenum_pull(WsManClient * cl, char *resourceUri,
		                  char *enumContext, actionOptions options);
	WsXmlDocH       wsenum_release(WsManClient * cl, char *resourceUri,
		                  char *enumContext, actionOptions options);
	WsXmlDocH       wsman_invoke(WsManClient * cl, char *resourceUri, char *method, void *data,
				                     actionOptions options);
	int             wsenum_enumerate_and_pull(WsManClient * cl, char *resource_uri,
				                      actionOptions options,
			                      SoapResponseCallback callback,
				                       void *callback_data);

	WsXmlDocH       wsman_client_create_request(WsManClient * cl,
					                 WsmanAction action, char *method, char *resource_uri,
			                 actionOptions options, void *data);
	char           *wsenum_get_enum_context(WsXmlDocH doc);
	void            wsman_add_fragment_transfer(WsXmlDocH doc, char *fragment);
	
	void            wsman_add_selector_from_uri(WsXmlDocH doc, char *resourceUri);



              /* Action options handling */
	void            initialize_action_options(actionOptions * op);
    void            destroy_action_options(actionOptions * op);
	void            wsman_add_selectors_from_query_string(actionOptions * options,
				                  const char *query_string);
	void            wsman_add_properties_from_query_string(actionOptions * options,
				                  const char *query_string);
	void            wsman_add_selector_from_options(WsXmlDocH doc, actionOptions options);
	void            wsman_set_action_option(actionOptions * options, unsigned int);
	void            wsman_set_options_from_uri(char *resourceUri, actionOptions * options);

	void            wsman_client_add_selector(actionOptions * options, char *key, char *value);
	void            wsman_client_add_property(actionOptions * options, char *key, char *value);

               /* Misc */
               
    /* Place holder */
	int				wsman_client_get_last_error(WsManClient * cl);
	void            wsman_remove_query_string(char *resourceUri, char **result);
	int 			wsman_client_check_for_fault(WsXmlDocH doc );

/*---------------- Advanced client API --------------------------------------*/

typedef struct {
	WsXmlDocH	request;
	WsXmlDocH	response;
	char		*fault_message;
	int		response_code;
} wsman_data_t;


int wsman_session_open(const char *server,
			int port,
			const char *path,
			const char *scheme,
			const char *username,
			const char *password);

void wsman_session_close(int session_id);

int wsman_session_uri_set(int session_id, const char *resource_uri);
char* wsman_session_resource_uri_get(int session_id);

WsXmlDocH wsman_session_request_create(int session_id, WsmanAction action);
wsman_data_t* wsman_session_request_send(int session_id, WsXmlDocH request);
wsman_data_t* wsman_session_do_action(int session_id, WsmanAction action);


#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* WSMANCLIENT_H_ */
