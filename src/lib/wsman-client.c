/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,cl
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
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGclE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Anas Nashif
 */

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <stdbool.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-dispatcher.h"
#include "wsman-soap-envelope.h"

#include "wsman-xml.h"
#include "wsman-xml-serialize.h"
#include "wsman-filter.h"
#include "wsman-client-transport.h"
#include "wsman-faults.h"
#include "wsman-client.h"

void
wsmc_set_dumpfile( WsManClient *cl, FILE *f )
{
    if (f)
		cl->dumpfile = f;
    return;
}


FILE *
wsmc_get_dumpfile(WsManClient *cl)
{
    return cl->dumpfile;
}


#ifndef _WIN32
void
wsmc_set_conffile(WsManClient *cl, char *f )
{
        u_free(cl->client_config_file);
        cl->client_config_file = (f != NULL) ? u_strdup(f): NULL;
}

char *
wsmc_get_conffile(WsManClient *cl)
{
        return cl->client_config_file;
}
#endif


static char*
wsman_make_action(const char *uri, const char *op_name)
{
	if (uri && op_name) {
		size_t len = strlen(uri) + strlen(op_name) + 2;
		char *ptr = (char *)u_malloc(len);
		if (ptr) {
			int ret = snprintf(ptr, len, "%s/%s", uri, op_name);
			if (ret < 0 || ret >= len) {
				error("Error: formatting action");
				u_free(ptr);
				return NULL;
			}
			return ptr;
		}
	}
	return NULL;
}


/*
 * find key in property list
 * callback for list_find()
 */
static int
_list_key_compare(const void *node1, const void *key)
{
  const char *key1 = ((key_value_t *)node1)->key;
  const char *key2 = (const char *)key;
  return strcmp(key1, key2);
}


static WsXmlDocH
wsmc_build_envelope(WsSerializerContextH serctx,
		const char *action,
		const char *reply_to_uri,
		const char *resource_uri,
		const char *to_uri,
		client_opt_t *options)
{
	WsXmlNodeH      node;
	char            uuidBuf[100];
	WsXmlNodeH      header;
	WsXmlDocH       doc = ws_xml_create_envelope();
	if (!doc) {
		error("Error while creating envelope");
		return NULL;
	}

	header = ws_xml_get_soap_header(doc);
	if (header == NULL) {
		return NULL;
	}
	generate_uuid(uuidBuf, sizeof(uuidBuf), 0);

	if (reply_to_uri == NULL) {
		reply_to_uri = WSA_TO_ANONYMOUS;
	}
	if (to_uri == NULL) {
		to_uri = WSA_TO_ANONYMOUS;
	}
	if (action != NULL) {
		ws_serialize_str(serctx, header,
			(char *)action, XML_NS_ADDRESSING, WSA_ACTION, 1);
	}

	if (to_uri) {
		ws_serialize_str(serctx, header, (char *)to_uri,
			XML_NS_ADDRESSING, WSA_TO, 1);
	}
	if (resource_uri) {
		ws_serialize_str(serctx, header, (char *)resource_uri,
				XML_NS_WS_MAN, WSM_RESOURCE_URI, 1);
	}
	if (uuidBuf[0] != 0) {
		ws_serialize_str(serctx, header, uuidBuf,
			XML_NS_ADDRESSING, WSA_MESSAGE_ID, 1);
	}
	if (options->timeout) {
	  /* FIXME: see wsman-xml-serialize.c */
		char buf[20];
		int ret;
		ret = snprintf(buf, sizeof(buf), "PT%u.%uS",
				(unsigned int) options->timeout / 1000,
				(unsigned int) options->timeout % 1000);
		if (ret < 0 || ret >= sizeof(buf)) {
			error("Error: formatting time");
			return NULL;
		}
		ws_serialize_str(serctx, header, buf,
			XML_NS_WS_MAN, WSM_OPERATION_TIMEOUT, 0);
	}
	if (options->max_envelope_size) {
		ws_serialize_uint32(serctx, header, options->max_envelope_size,
				XML_NS_WS_MAN, WSM_MAX_ENVELOPE_SIZE,
				options->flags & FLAG_MUND_MAX_ESIZE);
	}
	if (options->fragment) {
          /* FIXME: mu set but unused
		int mu = 0;
		if ((options->flags & FLAG_MUND_FRAGMENT) ==
				FLAG_MUND_FRAGMENT)
			mu = 1;
           */
		ws_serialize_str(serctx, header, options->fragment,
				XML_NS_WS_MAN, WSM_FRAGMENT_TRANSFER,
				1);
	}
	if (options->locale) {
          node = ws_xml_add_child(header, XML_NS_WS_MAN, WSM_LOCALE, NULL);
          ws_xml_set_node_lang(node, options->locale);
          ws_xml_add_node_attr(node, XML_NS_SOAP_1_2, SOAP_MUST_UNDERSTAND, "false");
	}

	node = ws_xml_add_child(header, XML_NS_ADDRESSING, WSA_REPLY_TO, NULL);
	if (node == NULL) {
		return NULL;
	}
	ws_xml_add_child(node, XML_NS_ADDRESSING, WSA_ADDRESS, (char *)reply_to_uri);

  	/* Do not add the selectors to the header for reference instances */
	if ((options->flags & FLAG_CIM_REFERENCES) != FLAG_CIM_REFERENCES) {
		wsmc_add_selector_from_options(doc, options);

		if (options->cim_ns) {
                  /* don't add CIM_NAMESPACE_SELECTOR twice */
                  if (options->selectors && list_count(options->selectors) > 0) {
                    if (!list_find(options->selectors, CIM_NAMESPACE_SELECTOR, _list_key_compare)) {
                      wsman_add_selector(header, CIM_NAMESPACE_SELECTOR, options->cim_ns);
                    }
                  }
                  else {
                    wsman_add_selector(header, CIM_NAMESPACE_SELECTOR, options->cim_ns);
                  }
                }
	}
	return doc;
}



// Access to client elements
WS_LASTERR_Code
wsmc_get_last_error(WsManClient * cl)
{
	return cl->last_error;
}


long
wsmc_get_response_code(WsManClient * cl)
{
	return cl->response_code;
}

char*
wsmc_get_fault_string(WsManClient * cl)
{
	return cl->fault_string;
}

WsSerializerContextH
wsmc_get_serialization_context(WsManClient * cl)
{
	return cl->serctx;
}


char *
wsmc_get_hostname(WsManClient * cl)
{
	return cl->data.hostname? u_strdup (cl->data.hostname) : NULL;
}


unsigned int
wsmc_get_port(WsManClient * cl)
{
	return cl->data.port;
}

char *
wsmc_get_scheme(WsManClient * cl)
{
	return cl->data.scheme ?  u_strdup ( cl->data.scheme) : NULL;
}


char *
wsmc_get_path(WsManClient * cl)
{
	return cl->data.path ?  u_strdup( cl->data.path ) : NULL;
}


char *
wsmc_get_user(WsManClient * cl)
{
	return  cl->data.user ? u_strdup( cl->data.user ) : NULL;
}


char *
wsmc_get_encoding(WsManClient *cl)
{
	return cl->content_encoding;
}


char *
wsmc_get_password(WsManClient * cl)
{
	return   cl->data.pwd ? u_strdup( cl->data.pwd ) : NULL;
}


char *
wsmc_get_endpoint(WsManClient * cl)
{
	return cl->data.endpoint ? cl->data.endpoint : NULL;
}




WsXmlDocH
wsmc_read_file( const char *filename,
		const char *encoding, unsigned long options)
{
	return ws_xml_read_file( filename, encoding, options);
}

WsXmlDocH
wsmc_read_memory( char *buf,
		size_t size, const char *encoding, unsigned long options)
{
	return ws_xml_read_memory( buf, size, encoding, options);
}

client_opt_t *
wsmc_options_init(void)
{
	client_opt_t *op = u_malloc(sizeof(client_opt_t));
	if (op)
		memset(op, 0, sizeof(client_opt_t));
	else
		return NULL;
	return op;
}

/*
 * Destroy list of key_value_t
 *
 */
static void
_wsmc_kvl_destroy(list_t *kvl)
{
  if (!kvl)
    return;
  while (!list_isempty(kvl)) {
    lnode_t *node;
    key_value_t *kv;

    node = list_del_last(kvl);
    if (!node)
      break;
    kv = (key_value_t *)node->list_data;
    lnode_destroy(node);
    key_value_destroy(kv, 0);
  }
  list_destroy(kvl);
}

void
wsmc_options_destroy(client_opt_t * op)
{
	if (op->options) {
		hash_free(op->options);
	}
        _wsmc_kvl_destroy(op->selectors);
        _wsmc_kvl_destroy(op->properties);
	u_free(op->fragment);
	u_free(op->cim_ns);
	u_free(op->delivery_uri);
	u_free(op->reference);
	u_free(op);
	return;
}

void
wsmc_set_action_option(client_opt_t * options, unsigned long flag)
{
	options->flags |= flag;
	return;
}


unsigned long
wsmc_get_action_option(client_opt_t * options)
{
	return options->flags;
}


void
wsmc_clear_action_option(client_opt_t * options, unsigned long flag)
{
	options->flags &= ~flag;
	return;
}


/*
 * (static function)
 * Add key/value to list_t (e.g. options->properties or ->selectors)
 * either as char* (string set, epr == NULL)
 * or as epr_t (string == NULL, epr set)
 * if (useArray == true) then value will be added without duplicate key checking
 */

static void
_wsmc_add_key_value(list_t **list_ptr,
		const char *key,
		const char *string,
		const epr_t *epr,
                bool useArray)
{
  key_value_t *kv;
  list_t *list = *list_ptr;
  lnode_t *lnode;
  if ((string != NULL) && (epr != NULL)) {
    error("Ambiguous call to add_key_value");
    return;
  }
  if (key == NULL) {
    error("Can't add_key_value with NULL key");
    return;
  }
  if (list == NULL) {
    list = list_create(LISTCOUNT_T_MAX);
    if (list == NULL) {
      error("list_create returned NULL");
      return;
    }
    *list_ptr = list;
  }
  if (!useArray && list_find(list, key, _list_key_compare)) {
    error("duplicate key not added to list");
    return;
  }
  kv = key_value_create(key, string, epr, NULL);
  if (!kv) {
    error("No memory for key/value pair");
    return;
  }
  lnode = lnode_create(kv);
  if (!lnode) {
    error("No memory for key/value node");
    return;
  }
  list_append(list, lnode);
}

/*
 * add a char* as an array type property (an array item)
 *
 */

void
wsmc_add_property_array_item(client_opt_t * options,
		const char *key,
		const char *value)
{
  _wsmc_add_key_value(&(options->properties), key, value, NULL, true);
}

/*
 * add a char* as property
 *
 */

void
wsmc_add_property(client_opt_t * options,
		const char *key,
		const char *value)
{
  _wsmc_add_key_value(&(options->properties), key, value, NULL, false);
}


/*
 * add an EndpointReference as property
 *
 */

void
wsmc_add_property_epr(client_opt_t * options,
		const char *key,
		const epr_t *value)
{
  _wsmc_add_key_value(&(options->properties), key, NULL, value, false);
}

/*
 * add key/value pair to WSM_OPTION_SET
 *
 */
void
wsmc_add_option(client_opt_t * options,
		const char *key,
		const char *value)
{
	if (options->options == NULL) {
		options->options = hash_create3(HASHCOUNT_T_MAX, 0, 0);
		if (options->options == NULL) {
			error("hash_create3 failed");
			return;
		}
	}
	if (!hash_lookup(options->options, key)) {
          char *k = u_strdup(key);
          char *v = u_strdup(value);
		if (!hash_alloc_insert(options->options, k, v)) {
			error( "hash_alloc_insert failed");
                  u_free(v);
                  u_free(k);
		}
	} else {
		error( "duplicate not added to hash");
	}
}

void
wsmc_add_selector(client_opt_t *options,
		const char *key,
		const char *value)
{
  _wsmc_add_key_value(&(options->selectors), key, value, NULL, false);
}

void
wsmc_add_selector_epr(client_opt_t *options,
		const char *key,
		const epr_t *value)
{
  _wsmc_add_key_value(&(options->selectors), key, NULL, value, false);
}

static void
_wsmc_add_uri_to_list(list_t **list_ptr, const char *query_string)
{
  hash_t *query;
  if (!query_string || !list_ptr)
    return;
  
  query = u_parse_query(query_string);
  if (query) {
    /* convert query hash to property list */
    hscan_t hs;
    hnode_t *hn;
    list_t *list = *list_ptr;
    _wsmc_kvl_destroy(list);
    *list_ptr = NULL;
    hash_scan_begin(&hs, query);
    while ((hn = hash_scan_next(&hs))) {
      _wsmc_add_key_value(list_ptr,
                          (char *)hnode_getkey(hn),
                          (char *)hnode_get(hn),
                          NULL, false);
    }
    hash_free(query);
  }
}

void
wsmc_add_selectors_from_str(client_opt_t * options,
		const char *query_string)
{
  _wsmc_add_uri_to_list(&(options->selectors), query_string);
}

void
wsmc_add_prop_from_str(client_opt_t * options,
		const char *query_string)
{
  _wsmc_add_uri_to_list(&(options->properties), query_string);
}

void
wsmc_add_selector_from_options(WsXmlDocH doc, client_opt_t *options)
{
	WsXmlNodeH      header;
	lnode_t        *node;
	if (!options->selectors || list_count(options->selectors) == 0)
		return;
	header = ws_xml_get_soap_header(doc);
	if (header == NULL) {
		return;
	}
        node = list_first(options->selectors);
  	while (node) {
          key_value_t *kv;
          kv = (key_value_t *)node->list_data;
          if (kv->type == 0) {
            wsman_add_selector(header, kv->key, kv->v.text);
          }
          else {
            wsman_add_selector_epr(header, kv->key, kv->v.epr);
          }
          node = list_next(options->selectors, node);
	}
}

void
wsmc_set_selectors_from_uri(const char *resource_uri, client_opt_t * options)
{
  if (options->selectors) {
    list_destroy_nodes(options->selectors);
    list_destroy(options->selectors);
    options->selectors = NULL;
  }
  u_uri_t *uri = u_malloc(sizeof(u_uri_t));
  u_uri_parse(resource_uri, &uri);
  wsmc_add_selectors_from_str(options, uri->query);
  u_uri_free(uri);
}

/* Err: should be wsmc_set_selectors_from_uri() */
void
wsmc_set_options_from_uri(const char *resource_uri, client_opt_t * options)
{
  error("Call to deprecated 'wsmc_set_options_from_uri', use 'wsmc_set_selectors_from_uri' instead");
  wsmc_set_selectors_from_uri(resource_uri, options);
}

void
wsmc_set_delivery_uri(const char *delivery_uri, client_opt_t * options)
{
  u_free(options->delivery_uri);
  options->delivery_uri = delivery_uri ? u_strdup(delivery_uri) : NULL;
}

void
wsmc_set_cim_ns(const char *ns, client_opt_t * options)
{
  u_free(options->cim_ns);
  options->cim_ns = ns ? u_strdup(ns) : NULL;
}

void
wsmc_set_fragment(const char *fragment, client_opt_t * options)
{
  u_free(options->fragment);
  options->fragment = fragment ? u_strdup(fragment) : NULL;
}

void
wsmc_set_reference(const char *reference, client_opt_t * options)
{
  u_free(options->reference);
  options->reference = reference ? u_strdup(reference) : NULL;
}

void
wsmc_set_sub_expiry(float event_subscription_expire, client_opt_t * options)
{
	options->expires = event_subscription_expire;
}

void
wsmc_set_heartbeat_interval(float heartbeat_interval, client_opt_t * options)
{
	options->heartbeat_interval = heartbeat_interval;
}

void
wsmc_set_delivery_mode(WsmanDeliveryMode delivery_mode, client_opt_t * options)
{
	options->delivery_mode = delivery_mode;
}

void
wsmc_set_delivery_security_mode(WsManDeliverySecurityMode delivery_sec_mode, client_opt_t * options)
{
        options->delivery_sec_mode = delivery_sec_mode;
}

void
wsmc_set_locale(client_opt_t * options, const char *locale)
{
  u_free(options->locale);
  options->locale = locale ? u_strdup(locale) : NULL;
}

char *
wsmc_get_locale(client_opt_t * options)
{
  return options->locale;
}

void
wsmc_add_selector_from_uri(WsXmlDocH doc,
		const char *resource_uri)
{
	u_uri_t        *uri = NULL;
	WsXmlNodeH      header = ws_xml_get_soap_header(doc);
	hash_t         *query;
	hnode_t        *hn;
	hscan_t         hs;

	if (header == NULL) {
		return;
	}
	if (resource_uri == NULL) {
		return;
	}
	if (u_uri_parse((const char *) resource_uri, &uri) != 0) {
		return;
	} else if (!uri->query) {
		goto cleanup;
	}

	query = u_parse_query(uri->query);
	if (query == NULL) {
		goto cleanup;
	}
	hash_scan_begin(&hs, query);
	while ((hn = hash_scan_next(&hs))) {
		wsman_add_selector(header,
				(char *) hnode_getkey(hn),
				(char *) hnode_get(hn));
		debug("key=%s value=%s", (char *) hnode_getkey(hn),
				(char *) hnode_get(hn));
	}
	hash_free_nodes(query);
	hash_destroy(query);
cleanup:
	if (uri) {
		u_uri_free(uri);
	}
}


static char*
wsmc_create_action_str(WsmanAction action)
{
	char           *action_str = NULL;

	switch (action) {
	case WSMAN_ACTION_ASSOCIATORS:
	case WSMAN_ACTION_REFERENCES:
	case WSMAN_ACTION_ENUMERATION:
		action_str = wsman_make_action(XML_NS_ENUMERATION, WSENUM_ENUMERATE);
		break;
	case WSMAN_ACTION_PULL:
		action_str = wsman_make_action(XML_NS_ENUMERATION, WSENUM_PULL);
		break;
	case WSMAN_ACTION_RELEASE:
		action_str = wsman_make_action(XML_NS_ENUMERATION, WSENUM_RELEASE);
		break;
	case WSMAN_ACTION_TRANSFER_CREATE:
		action_str = wsman_make_action(XML_NS_TRANSFER, TRANSFER_CREATE);
		break;
	case WSMAN_ACTION_TRANSFER_DELETE:
		action_str = wsman_make_action(XML_NS_TRANSFER, TRANSFER_DELETE);
		break;
	case WSMAN_ACTION_TRANSFER_GET:
		action_str = wsman_make_action(XML_NS_TRANSFER, TRANSFER_GET);
		break;
	case WSMAN_ACTION_TRANSFER_PUT:
		action_str = wsman_make_action(XML_NS_TRANSFER, TRANSFER_PUT);
		break;
	case WSMAN_ACTION_SUBSCRIBE:
		action_str = wsman_make_action(XML_NS_EVENTING, WSEVENT_SUBSCRIBE);
		break;
	case WSMAN_ACTION_UNSUBSCRIBE:
		action_str = wsman_make_action(XML_NS_EVENTING, WSEVENT_UNSUBSCRIBE);
		break;
	case WSMAN_ACTION_RENEW:
		action_str = wsman_make_action(XML_NS_EVENTING, WSEVENT_RENEW);
		break;
	case WSMAN_ACTION_NONE:
	case WSMAN_ACTION_IDENTIFY:
	case WSMAN_ACTION_ANON_IDENTIFY:
	case WSMAN_ACTION_TEST:
	case WSMAN_ACTION_CUSTOM:
		break;
	}
	return action_str;
}

static char *
wsmc_create_delivery_mode_str(WsmanDeliveryMode mode)
{
	char * str = NULL;
	switch(mode) {
		case WSMAN_DELIVERY_PUSH:
			str = u_strdup(WSEVENT_DELIVERY_MODE_PUSH);
			break;
		case WSMAN_DELIVERY_PUSHWITHACK:
			str = u_strdup(WSEVENT_DELIVERY_MODE_PUSHWITHACK);
			break;
		case WSMAN_DELIVERY_EVENTS:
			str = u_strdup(WSEVENT_DELIVERY_MODE_EVENTS);
			break;
		case WSMAN_DELIVERY_PULL:
			str = u_strdup(WSEVENT_DELIVERY_MODE_PULL);
	}
	return str;
}

static char *
wsmc_create_delivery_sec_mode_str(WsManDeliverySecurityMode mode)
{
	char * str = NULL;
	switch(mode) {
		case WSMAN_DELIVERY_SEC_HTTP_BASIC:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTP_BASIC);
			break;
		case WSMAN_DELIVERY_SEC_HTTP_DIGEST:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTP_DIGEST);
			break;
		case WSMAN_DELIVERY_SEC_HTTPS_BASIC:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTPS_BASIC);
			break;
		case WSMAN_DELIVERY_SEC_HTTPS_DIGEST:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTPS_DIGEST);
			break;
		case WSMAN_DELIVERY_SEC_HTTPS_MUTUAL:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL);
			break;
		case WSMAN_DELIVERY_SEC_HTTPS_MUTUAL_BASIC:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_BASIC);
			break;
		case WSMAN_DELIVERY_SEC_HTTPS_MUTUAL_DIGEST:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_DIGEST);
			break;
		case WSMAN_DELIVERY_SEC_HTTPS_SPNEGO_KERBEROS:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTPS_SPNEGO_KERBEROS);
			break;
		case WSMAN_DELIVERY_SEC_HTTPS_MUTUAL_SPNEGO_KERBEROS:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTPS_MUTUAL_SPNEGO_KERBEROS);
			break;
		case WSMAN_DELIVERY_SEC_HTTP_SPNEGO_KERBEROS:
			str = u_strdup(WSMAN_SECURITY_PROFILE_HTTP_SPNEGO_KERBEROS);
			break;
		default:
			break;
	}
	return str;
}

static void
wsman_set_enumeration_options(WsManClient * cl, WsXmlNodeH body, const char* resource_uri,
			client_opt_t *options, filter_t *filter)
{
	WsXmlNodeH node = ws_xml_get_child(body, 0, NULL, NULL);
	if (node == NULL) {
		return;
	}
	if ((options->flags & FLAG_ENUMERATION_OPTIMIZATION) ==
			FLAG_ENUMERATION_OPTIMIZATION) {
		ws_xml_add_child(node, XML_NS_WS_MAN, WSM_OPTIMIZE_ENUM, NULL);
	}

	if ((options->flags & FLAG_ENUMERATION_ENUM_EPR) ==
			FLAG_ENUMERATION_ENUM_EPR) {
		ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE, WSM_ENUM_EPR);
	} else if ((options->flags & FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) ==
			FLAG_ENUMERATION_ENUM_OBJ_AND_EPR) {
		ws_xml_add_child(node, XML_NS_WS_MAN, WSM_ENUM_MODE,
				WSM_ENUM_OBJ_AND_EPR);
	}

	// Polymorphism
	if ((options->flags & FLAG_IncludeSubClassProperties)
			== FLAG_IncludeSubClassProperties) {
		ws_xml_add_child(node, XML_NS_CIM_BINDING,
				WSMB_POLYMORPHISM_MODE, WSMB_INCLUDE_SUBCLASS_PROP);
	} else if ((options->flags & FLAG_ExcludeSubClassProperties) ==
			FLAG_ExcludeSubClassProperties) {
		ws_xml_add_child(node, XML_NS_CIM_BINDING,
				WSMB_POLYMORPHISM_MODE, WSMB_EXCLUDE_SUBCLASS_PROP);
	} else if ((options->flags & FLAG_POLYMORPHISM_NONE)
			== FLAG_POLYMORPHISM_NONE) {
		ws_xml_add_child(node, XML_NS_CIM_BINDING,
				WSMB_POLYMORPHISM_MODE, "None");
	}

	if (filter != NULL) {
		filter_serialize(node, filter, XML_NS_WS_MAN);
	}
	return;
}

static void
wsman_set_subscribe_options(WsManClient * cl,
			WsXmlDocH request,
			const char* resource_uri,
			client_opt_t *options,
			filter_t *filter)
{
	WsXmlNodeH body = ws_xml_get_soap_body(request);
	WsXmlNodeH header = ws_xml_get_soap_header(request);
	WsXmlNodeH node = NULL, temp = NULL, node2 = NULL, node3 = NULL;
	char buf[32];

	if (header == NULL || body == NULL) {
		return;
	}
	if(options->delivery_certificatethumbprint ||
	   options->delivery_username ||
	   options->delivery_password) {
		node = ws_xml_add_child(header, XML_NS_TRUST, WST_ISSUEDTOKENS, NULL);
		if (node == NULL) {
			return;
		}
		ws_xml_add_node_attr(node, XML_NS_SOAP_1_2, SOAP_MUST_UNDERSTAND, "true");
		if(options->delivery_certificatethumbprint) {
			if (node == NULL) {
				return;
			}
			node2 = ws_xml_add_child(node, XML_NS_TRUST, WST_REQUESTSECURITYTOKENRESPONSE, NULL);
			if (node2 == NULL) {
				return;
			}
			ws_xml_add_child(node2, XML_NS_TRUST, WST_TOKENTYPE,WST_CERTIFICATETHUMBPRINT );
			node3 = ws_xml_add_child(node2, XML_NS_TRUST, WST_REQUESTEDSECURITYTOKEN, NULL);
			if (node3 == NULL) {
				return;
			}
			ws_xml_add_child(node3, XML_NS_WS_MAN, WSM_CERTIFICATETHUMBPRINT,
				options->delivery_certificatethumbprint);
			node3 = ws_xml_add_child(node2, XML_NS_POLICY, WSP_APPLIESTO, NULL);
			if (node3 == NULL) {
				return;
			}
			node3 = ws_xml_add_child(node3, XML_NS_ADDRESSING, WSA_EPR, NULL);
			if (node3 == NULL) {
				return;
			}
			ws_xml_add_child(node3, XML_NS_ADDRESSING, WSA_ADDRESS, options->delivery_uri);
		}
		if(options->delivery_username || options->delivery_password) {
			node2 = ws_xml_add_child(node, XML_NS_TRUST, WST_REQUESTSECURITYTOKENRESPONSE, NULL);
			if (node2 == NULL) {
				return;
			}
			ws_xml_add_child(node2, XML_NS_TRUST, WST_TOKENTYPE,WST_USERNAMETOKEN);
			node3 = ws_xml_add_child(node2, XML_NS_TRUST, WST_REQUESTEDSECURITYTOKEN, NULL);
			if (node3 == NULL) {
				return;
			}
			node3 = ws_xml_add_child(node3, XML_NS_SE, WSSE_USERNAMETOKEN, NULL);
			if (options->delivery_username) {
				if (node3 == NULL) {
					return;
				}
				ws_xml_add_child(node3, XML_NS_SE, WSSE_USERNAME, options->delivery_username);
			}
			if (options->delivery_password) {
				if (node3 == NULL) {
					return;
				}
				ws_xml_add_child(node3, XML_NS_SE, WSSE_PASSWORD, options->delivery_password);
			}
			node3 = ws_xml_add_child(node2, XML_NS_POLICY, WSP_APPLIESTO, NULL);
			if (node3 == NULL) {
				return;
			}
			node3 = ws_xml_add_child(node3, XML_NS_ADDRESSING, WSA_EPR, NULL);
			if (node3 == NULL) {
				return;
			}
			ws_xml_add_child(node3, XML_NS_ADDRESSING, WSA_ADDRESS, options->delivery_uri);
		}
	}
	node = ws_xml_add_child(body,
				XML_NS_EVENTING, WSEVENT_SUBSCRIBE,NULL);
	if (node == NULL) {
		return;
	}
	temp = ws_xml_add_child(node, XML_NS_EVENTING, WSEVENT_DELIVERY, NULL);
	if(temp) {
		char *mode = wsmc_create_delivery_mode_str(options->delivery_mode);
		ws_xml_add_node_attr(temp, NULL, WSEVENT_DELIVERY_MODE, mode);
		u_free(mode);
		if(options->delivery_uri) {
			node2 = ws_xml_add_child(temp, XML_NS_EVENTING, WSEVENT_NOTIFY_TO, NULL);
			if (node2 == NULL) {
				return;
			}
			ws_xml_add_child(node2, XML_NS_ADDRESSING, WSA_ADDRESS, options->delivery_uri);
		}
		if(options->delivery_sec_mode) {
			temp = ws_xml_add_child(temp, XML_NS_WS_MAN, WSM_AUTH, NULL);
			if (temp == NULL) {
				return;
			}
			char *mode = wsmc_create_delivery_sec_mode_str(options->delivery_sec_mode);
			ws_xml_add_node_attr(temp, NULL, WSM_PROFILE, mode);
			u_free(mode);
		}
		if(options->heartbeat_interval) {
			snprintf(buf, 32, "PT%fS", options->heartbeat_interval);
			ws_xml_add_child(temp, XML_NS_WS_MAN, WSM_HEARTBEATS, buf);
		}
		if(options->reference) {
			WsXmlDocH doc = ws_xml_read_memory( options->reference, strlen(options->reference), "UTF-8", 0);
			node3 = ws_xml_get_doc_root(doc);
			temp = ws_xml_add_child(node2, XML_NS_ADDRESSING, WSA_REFERENCE_PROPERTIES, NULL);
			if(temp)
				ws_xml_duplicate_tree(temp, node3);
		}
	}
	if (options->expires) {
		snprintf(buf, 32, "PT%fS", options->expires);
		ws_xml_add_child(node, XML_NS_EVENTING, WSEVENT_EXPIRES, buf);
	}
	if (filter) {
		filter_serialize(node, filter, XML_NS_WS_MAN);
	}
	if (options->flags & FLAG_EVENT_SENDBOOKMARK) {
		ws_xml_add_child(node, XML_NS_WS_MAN, WSM_SENDBOOKMARKS, NULL);
	}
}

static void
wsmc_set_put_prop(WsXmlDocH get_response,
		WsXmlDocH put_request,
		client_opt_t *options)
{
	WsXmlNodeH      resource_node;
	char           *ns_uri;
	WsXmlNodeH      get_body = ws_xml_get_soap_body(get_response);
	WsXmlNodeH      put_body = ws_xml_get_soap_body(put_request);

	ws_xml_copy_node(ws_xml_get_child(get_body, 0, NULL, NULL), put_body);
	resource_node = ws_xml_get_child(put_body, 0, NULL, NULL);
	ns_uri = ws_xml_get_node_name_ns_uri(resource_node);

	if (!list_isempty(options->properties)) {
		lnode_t *node = list_first(options->properties);
		while (node) {
			key_value_t *property = (key_value_t *)node->list_data;
			WsXmlNodeH n = ws_xml_get_child(resource_node, 0, ns_uri, property->key);
			if (n != NULL) {
				if (property->type == 0) {
					ws_xml_set_node_text(n, property->v.text);
				}
				else {
					epr_serialize(n, ns_uri, property->key, property->v.epr, 1);
				}
			}
			node = list_next(options->properties, node);
		}
	}
}


void
wsmc_node_to_buf(WsXmlNodeH node, char **buf) {
	int   len;
	WsXmlDocH doc = ws_xml_create_doc_by_import(node);
	if (doc == NULL) {
		return;
	}
	ws_xml_dump_memory_enc(doc, buf, &len, "UTF-8");
	ws_xml_destroy_doc(doc);
	return;
}


char*
wsmc_node_to_formatbuf(WsXmlNodeH node) {
	char *buf;
	int   len;
	WsXmlDocH doc = ws_xml_create_doc_by_import(node);
	if (doc == NULL) {
		return NULL;
	}
	WsXmlNodeH root = ws_xml_get_doc_root(doc);
	if (root == NULL) {
		return NULL;
	}
	ws_xml_dump_memory_node_tree(root, &buf, &len);
	ws_xml_destroy_doc(doc);
	return buf;
}


static void
add_subscription_context(WsXmlNodeH node, char *context)
{
	WsXmlNodeH subsnode;
	WsXmlDocH doc = ws_xml_read_memory(context, strlen(context), "UTF-8", 0);
	if (doc == NULL) {
		return;
	}
	subsnode = ws_xml_get_doc_root(doc);
	ws_xml_duplicate_children(node, subsnode);
}

WsXmlDocH
wsmc_create_request(WsManClient * cl, const char *resource_uri,
		client_opt_t *options, filter_t *filter,
		WsmanAction action, char *method, void *data)
{
	WsXmlDocH       request;
	WsXmlNodeH      body;
	WsXmlNodeH      header;
	WsXmlNodeH      node;
	char           *_action = NULL;
	if (action == WSMAN_ACTION_IDENTIFY) {
		request = ws_xml_create_envelope();
	} else {
		if (method) {
		        /* WSA_ACTION is a URI */
			if (strchr(method, '/'))
				_action = u_strdup(method);
			else /* 'invoke' action */
				_action = wsman_make_action((char *)resource_uri, method);
		} else {
			_action = wsmc_create_action_str(action);
		}
		if (options && _action) {
			request = wsmc_build_envelope(cl->serctx, _action,
					WSA_TO_ANONYMOUS, (char *)resource_uri,
					cl->data.endpoint, options);
		} else {
			u_free(_action);
			return NULL;
		}
		u_free(_action);
	}

	body = ws_xml_get_soap_body(request);
	header = ws_xml_get_soap_header(request);
	if (!body  || !header )
		return NULL;
	/*
	 * flags to be passed to client
	 */
	if (options) {
		if (options->flags & FLAG_SUPRESS_100_CONTINUE) {
			cl->flags |= WSMAN_CLIENT_SUPRESS_100_CONTINUE;
		} else {
			cl->flags &= ~WSMAN_CLIENT_SUPRESS_100_CONTINUE;
		}
	}
	/*
	 * flags to be passed as <w:OptionSet ...> <w:Option Name="..." ...> >
	 */
	if (options) {
	  WsXmlNodeH opset = NULL;
	  if (options->flags & (FLAG_CIM_EXTENSIONS|FLAG_EXCLUDE_NIL_PROPS)) {
	    /* need WSM_OPTION_SET */
		opset = ws_xml_add_child(header,
				XML_NS_WS_MAN, WSM_OPTION_SET, NULL);
		if (opset == NULL) {
		  return NULL;
		}
		if ((options->flags & FLAG_CIM_EXTENSIONS) == FLAG_CIM_EXTENSIONS) {
			WsXmlNodeH op = ws_xml_add_child(opset,
				XML_NS_WS_MAN, WSM_OPTION, NULL);
			ws_xml_add_node_attr(op, NULL, WSM_NAME, WSMB_SHOW_EXTENSION);
		}
		if ((options->flags & FLAG_EXCLUDE_NIL_PROPS) == FLAG_EXCLUDE_NIL_PROPS) {
			/* ExcludeNilProperties is non-standard, so put it under an openwsman namespace */
			WsXmlNodeH op = ws_xml_add_child(opset,
				XML_NS_OPENWSMAN, WSM_OPTION, NULL);
			ws_xml_add_node_attr(op, NULL, WSM_NAME, WSMB_EXCLUDE_NIL_PROPS);
		}
	  }
	  if (options->options && hash_count(options->options) > 0) {
	    hnode_t *hn;
	    hscan_t hs;
	    hash_scan_begin(&hs, options->options);
	    if (!opset) {
	      opset = ws_xml_add_child(header,
				       XML_NS_WS_MAN, WSM_OPTION_SET, NULL);
	    }
	    while ((hn = hash_scan_next(&hs))) {
		  if (opset == NULL) {
			  return NULL;
		  }
	      WsXmlNodeH op = ws_xml_add_child(opset,
					       XML_NS_WS_MAN, WSM_OPTION, NULL);
	      ws_xml_add_node_attr(op, NULL, WSM_NAME, (char *) hnode_getkey(hn));
	      ws_xml_set_node_text(op, (char *) hnode_get(hn));
	    }
	  }
	}


	switch (action) {
	case WSMAN_ACTION_IDENTIFY:
	case WSMAN_ACTION_ANON_IDENTIFY:
		ws_xml_add_child(body,
				XML_NS_WSMAN_ID, WSMID_IDENTIFY, NULL);
		break;
	case WSMAN_ACTION_CUSTOM:
		break;
	case WSMAN_ACTION_ENUMERATION:
	case WSMAN_ACTION_ASSOCIATORS:
	case WSMAN_ACTION_REFERENCES:
		node = ws_xml_add_child(body,
				XML_NS_ENUMERATION, WSENUM_ENUMERATE, NULL);
		if (options) {
			wsman_set_enumeration_options(cl, body, resource_uri, options, filter);
		}
		break;
	case WSMAN_ACTION_PULL:
		node = ws_xml_add_child(body,
				XML_NS_ENUMERATION, WSENUM_PULL, NULL);
		if (node == NULL) {
		  return NULL;
		}
		if (data) {
			ws_xml_add_child(node, XML_NS_ENUMERATION,
					WSENUM_ENUMERATION_CONTEXT, (char *) data);
		}
		break;
	case WSMAN_ACTION_RELEASE:
		node = ws_xml_add_child(body,
				XML_NS_ENUMERATION, WSENUM_RELEASE, NULL);
		if (node == NULL) {
		  return NULL;
		}
		if (data) {
			ws_xml_add_child(node, XML_NS_ENUMERATION,
					WSENUM_ENUMERATION_CONTEXT, (char *) data);
		}
		break;
	case WSMAN_ACTION_SUBSCRIBE:
		if (options) {
			wsman_set_subscribe_options(cl, request, resource_uri, options, filter);
		}
		break;
	case WSMAN_ACTION_UNSUBSCRIBE:
		node = ws_xml_add_child(body,
				XML_NS_EVENTING, WSEVENT_UNSUBSCRIBE,NULL);
		if(data) {
			if(((char *)data)[0] != 0)
				add_subscription_context(ws_xml_get_soap_header(request), (char *)data);
		}
		break;
	case WSMAN_ACTION_RENEW:
          {
                char buf[20];
		node = ws_xml_add_child(body,
				XML_NS_EVENTING, WSEVENT_RENEW, NULL);
		if (node == NULL) {
			return NULL;
		}
                /* %f default precision is 6 -> [-]ddd.ddd */
		snprintf(buf, 20, "PT%fS", (options) ? options->expires : 0);
		ws_xml_add_child(node, XML_NS_EVENTING, WSEVENT_EXPIRES, buf);
		if(data) {
			if(((char *)data)[0] != 0)
				add_subscription_context(ws_xml_get_soap_header(request), (char *)data);
		}
          }
		break;
	case WSMAN_ACTION_NONE:
	case WSMAN_ACTION_TRANSFER_CREATE:
	case WSMAN_ACTION_TEST:
	case WSMAN_ACTION_TRANSFER_GET:
	case WSMAN_ACTION_TRANSFER_PUT:
	case WSMAN_ACTION_TRANSFER_DELETE:
		break;
	}

	if (action == WSMAN_ACTION_PULL || action == WSMAN_ACTION_ENUMERATION) {
		if (options && options->max_elements > 0 ) {
			node = ws_xml_get_child(body, 0, NULL, NULL);
			if (action == WSMAN_ACTION_ENUMERATION) {
				if ((options->flags & FLAG_ENUMERATION_OPTIMIZATION) ==
					FLAG_ENUMERATION_OPTIMIZATION ) {
						if (node == NULL) {
							return NULL;
						}
						/* wsman:MaxElements is for Enumerate */
						ws_xml_add_child_format(node, XML_NS_WS_MAN,
									WSENUM_MAX_ELEMENTS, "%d", options->max_elements);
				}
		         } else {
				if (node == NULL) {
					return NULL;
				}
				/* wsen:MaxElements is for Pull */
				ws_xml_add_child_format(node, XML_NS_ENUMERATION,
						WSENUM_MAX_ELEMENTS, "%d", options->max_elements);
			}
		}
		if (options && (options->flags & FLAG_ENUMERATION_COUNT_ESTIMATION) ==
				FLAG_ENUMERATION_COUNT_ESTIMATION) {
			ws_xml_add_child(header, XML_NS_WS_MAN, WSM_REQUEST_TOTAL, NULL);
		}
	}
	if (action != WSMAN_ACTION_TRANSFER_CREATE &&
			action != WSMAN_ACTION_TRANSFER_PUT &&
			action != WSMAN_ACTION_CUSTOM) {
		if (options && (options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
			ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
		}
	}
	return request;
}



static void
handle_resource_request(WsManClient * cl, WsXmlDocH request,
		void *data,
		void *typeInfo,
		char *resource_uri)
{
	if (data && typeInfo) {
		char           *class = u_strdup(strrchr(resource_uri, '/') + 1);
		ws_serialize(cl->serctx, ws_xml_get_soap_body(request),
				data, (XmlSerializerInfo *) typeInfo,
				class, resource_uri, NULL, 1);
		ws_serializer_free_mem(cl->serctx, data,
				(XmlSerializerInfo *) typeInfo);
		u_free(class);
	} else if (data != NULL) {
		if (wsman_is_valid_xml_envelope((WsXmlDocH) data)) {
			WsXmlNodeH      body = ws_xml_get_soap_body((WsXmlDocH) data);
			ws_xml_duplicate_tree(ws_xml_get_soap_body(request),
					ws_xml_get_child(body, 0, NULL, NULL));
		} else {
			ws_xml_duplicate_tree(ws_xml_get_soap_body(request),
					ws_xml_get_doc_root((WsXmlDocH) data));
		}
	}
}


static          WsXmlDocH
_wsmc_action_create(WsManClient * cl,
		char *resource_uri,
		void *data,
		void *typeInfo,
		client_opt_t *options)
{
	WsXmlDocH response;
	WsXmlDocH request = wsmc_create_request(cl, (char *)resource_uri, options, NULL,
			WSMAN_ACTION_TRANSFER_CREATE, NULL, NULL);
	if (!request)
		return NULL;

	handle_resource_request(cl, request, data, typeInfo, (char *)resource_uri);

	if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
		ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
	}
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}

WsXmlDocH
wsmc_action_create(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		WsXmlDocH source_doc)
{

	return _wsmc_action_create(cl, (char *)resource_uri,
			source_doc, NULL, options);
}


WsXmlDocH
wsmc_action_create_fromtext(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		const char *data, size_t size, const char *encoding)
{
	WsXmlDocH source_doc = wsmc_read_memory( (char *)data, size,
			(char *)encoding, 0);
	WsXmlDocH response;
	if (source_doc == NULL) {
		error("could not convert XML text to doc");
		return NULL;
	}

	response = _wsmc_action_create(cl, (char *)resource_uri,
			source_doc, NULL, options);
	ws_xml_destroy_doc(source_doc);
	return response;
}

WsXmlDocH
wsmc_action_create_serialized(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		void *data,
		void *typeInfo)
{
	return _wsmc_action_create(cl, (char *)resource_uri, data, typeInfo, options);
}


static WsXmlDocH
_wsmc_action_put(WsManClient * cl,
		char *resource_uri,
		void *data,
		void *typeInfo,
		client_opt_t *options)
{
	WsXmlDocH response;
	WsXmlDocH request = wsmc_create_request(cl, resource_uri, options, NULL,
			WSMAN_ACTION_TRANSFER_PUT, NULL, NULL);
	if (!request)
		return NULL;

	handle_resource_request(cl, request, data, typeInfo, resource_uri);
	if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
		ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
	}
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}

WsXmlDocH
wsmc_action_put(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		WsXmlDocH source_doc)
{
	return _wsmc_action_put(cl, (char *)resource_uri, source_doc, NULL, options);
}


WsXmlDocH
wsmc_action_put_fromtext(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		const char *data, size_t size, const char *encoding)
{
	WsXmlDocH source_doc = wsmc_read_memory( (char *)data, size,
			(char *)encoding, 0);
	WsXmlDocH response;
	if (source_doc == NULL) {
		error("could not convert XML text to doc");
		return NULL;
	}
	response =  _wsmc_action_put(cl, (char *)resource_uri, source_doc, NULL, options);
	ws_xml_destroy_doc(source_doc);
	return response;
}


WsXmlDocH
wsmc_action_put_serialized(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		void *data,
		void *typeInfo)
{
	return _wsmc_action_put(cl, (char *)resource_uri, data, typeInfo, options);
}

WsXmlDocH
wsmc_action_delete(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options)
{

	WsXmlDocH response;
	WsXmlDocH request = wsmc_create_request(cl, resource_uri, options, NULL,
				WSMAN_ACTION_TRANSFER_DELETE, NULL, NULL);
	if (!request)
		return NULL;

	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}

static int add_selectors_from_epr(void *options, const key_value_t *kv)
{
	client_opt_t *op = (client_opt_t *)options;
        if (kv->type == 0)
	  wsmc_add_selector(op, kv->key, kv->v.text);
        else
	  wsmc_add_selector_epr(op, kv->key, kv->v.epr);
	return 0;
}

WsXmlDocH
wsmc_action_delete_from_epr(WsManClient *cl, epr_t *epr,
		client_opt_t *options)
{
	char *resource_uri = epr_get_resource_uri(epr);
	wsman_epr_selector_cb(epr, add_selectors_from_epr, options);
	return wsmc_action_delete(cl, resource_uri, options);
}

WsXmlDocH
wsmc_action_get_from_epr(WsManClient *cl, epr_t *epr,
		client_opt_t *options)
{
	char *resource_uri = epr_get_resource_uri(epr);
	wsman_epr_selector_cb(epr, add_selectors_from_epr, options);
	return wsmc_action_get(cl, resource_uri, options);
}

WsXmlDocH
wsmc_action_get(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsmc_create_request(cl, resource_uri, options, NULL,
			WSMAN_ACTION_TRANSFER_GET, NULL, NULL);
	if (!request)
		return NULL;
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
wsmc_action_get_and_put(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options)
{
	WsXmlDocH       put_request;
	WsXmlDocH       put_response;
	WsXmlDocH       get_response = wsmc_action_get(cl, resource_uri, options);

	if (!get_response) {
		error("wsmc_action_get returned NULL doc");
		return NULL;
	}
	if (wsman_is_fault_envelope(get_response)) {
		return get_response;
	}
	put_request = wsmc_create_request(cl, resource_uri, options, NULL,
			WSMAN_ACTION_TRANSFER_PUT, NULL, (void *) get_response);
	if (!put_request)
		return NULL;

	wsmc_set_put_prop(get_response, put_request, options);
	//ws_xml_destroy_doc(get_response);
	if (wsman_send_request(cl, put_request)) {
		ws_xml_destroy_doc(put_request);
		return NULL;
	}
	put_response = wsmc_build_envelope_from_response(cl);

	//ws_xml_destroy_doc(put_request);
	return put_response;
}




WsXmlDocH
wsmc_action_invoke(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		const char *method,
		WsXmlDocH data)
{
	WsXmlDocH       response;
	WsXmlNodeH body = NULL;
        WsXmlDocH       request = wsmc_create_request(cl, resource_uri, options, NULL,
                WSMAN_ACTION_CUSTOM, (char *)method, NULL);
	if (!request)
		return NULL;

	body = ws_xml_get_soap_body(request);
	if (body == NULL) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	if (list_isempty(options->properties) &&
                data != NULL) {

		WsXmlNodeH n = ws_xml_get_doc_root(data);
		ws_xml_duplicate_tree(ws_xml_get_soap_body(request), n);
        } else if (!list_isempty(options->properties)) {
            if (method) {
              WsXmlNodeH xnode = ws_xml_add_empty_child_format(body,
                                  (char *)resource_uri, "%s_INPUT", method);
              if (xnode == NULL) {
                  return NULL;
              }
              lnode_t *lnode = list_first(options->properties);
              while (lnode) {
                key_value_t *property = (key_value_t *)lnode->list_data;
                if (property->type == 0) {
                  ws_xml_add_child(xnode, (char *)resource_uri, (char *)property->key, (char *)property->v.text);
                }
                else {
                  epr_serialize(xnode, (char *)resource_uri, property->key, property->v.epr, 1);
                }
                lnode = list_next(options->properties, lnode);
              }
            }
        } else if (!strchr(method, '/')) { /* non-custom method without parameters */
            ws_xml_add_empty_child_format(body,
                    (char *)resource_uri, "%s_INPUT", method);
        } else {
	  /* empty body */
	}
	if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
		ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
	}

	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
wsmc_action_invoke_fromtext(WsManClient * cl,
		const char *resourceUri,
		client_opt_t *options,
		const char *method,
		const char *data,
                size_t size,
                const char *encoding)
{
	WsXmlDocH       response;
        WsXmlDocH request = wsmc_create_request(cl, resourceUri, options, NULL,
						  WSMAN_ACTION_CUSTOM,
						  (char *)method, NULL);
	if (request == NULL) {
		error("could not create request");
		return NULL;
	}
	if ( data != NULL) {
		WsXmlDocH doc = wsmc_read_memory(
				(char *)data, size, (char *)encoding, 0);
		WsXmlNodeH n;
		if (doc == NULL) {
			error("could not wsmc_read_memory");
			ws_xml_destroy_doc(request);
			return NULL;
		}
		n = ws_xml_get_doc_root(doc);
		ws_xml_duplicate_tree(ws_xml_get_soap_body(request), n);
		ws_xml_destroy_doc(doc);
	} else {
            _warning("No XML provided");
        }
	if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
		ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
	}


	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}



WsXmlDocH
wsmc_action_invoke_serialized(WsManClient * cl,
		const char *resourceUri,
		client_opt_t *options,
		const char *method,
		void *typeInfo, void *data)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsmc_create_request(cl, resourceUri, options, NULL,
			WSMAN_ACTION_CUSTOM, (char *)method, NULL);
	if (request == NULL) {
		error("could not create request");
		return NULL;
	}
	if (data != NULL) {
		handle_resource_request(cl, request, data, typeInfo, (char *)resourceUri);
	}
	if ((options->flags & FLAG_DUMP_REQUEST) == FLAG_DUMP_REQUEST) {
		ws_xml_dump_node_tree(cl->dumpfile, ws_xml_get_doc_root(request));
	}


	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
wsmc_action_identify(WsManClient * cl,
		client_opt_t *options)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsmc_create_request(cl, NULL, options, NULL,
			WSMAN_ACTION_IDENTIFY, NULL, NULL);
	if (!request)
		return NULL;
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


int
wsmc_action_enumerate_and_pull(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		filter_t *filter,
		SoapResponseCallback callback,
		void *callback_data)
{
	WsXmlDocH doc;
	char *enumContext = NULL;
	WsXmlDocH enum_response = wsmc_action_enumerate(cl,
			resource_uri, options, filter);

	if (enum_response) {
		long rc = wsmc_get_response_code(cl);
		if (rc == 200 || rc == 400 || rc == 500) {
			callback(cl, enum_response, callback_data);
		} else {
			return 0;
		}
		enumContext = wsmc_get_enum_context(enum_response);
		ws_xml_destroy_doc(enum_response);
	} else {
		return 0;
	}

	while (enumContext != NULL && enumContext[0] != 0) {
		long rc = wsmc_get_response_code(cl);
		doc = wsmc_action_pull(cl, resource_uri, options, filter, enumContext);

		if (rc != 200 && rc != 400 && rc != 500) {
			wsmc_free_enum_context(enumContext);
			return 0;
		}
		callback(cl, doc, callback_data);
		wsmc_free_enum_context(enumContext);
		enumContext = wsmc_get_enum_context(doc);
		if (doc) {
			ws_xml_destroy_doc(doc);
		}
	}
	wsmc_free_enum_context(enumContext);
	return 1;
}




WsXmlDocH
wsmc_action_enumerate(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		filter_t *filter)
{
	WsXmlDocH response;
	WsXmlDocH request = wsmc_create_request(cl,
					resource_uri, options, filter,
					WSMAN_ACTION_ENUMERATION, NULL, NULL);
	if (!request)
		return NULL;
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}


WsXmlDocH
wsmc_action_pull(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		filter_t *filter,
		const char *enumContext)
{
	WsXmlDocH       response;
	WsXmlNodeH      node;

	if (enumContext || (enumContext && enumContext[0] == 0)) {
		WsXmlDocH request = wsmc_create_request(cl, resource_uri, options, filter,
				WSMAN_ACTION_PULL,	NULL, (char *)enumContext);
		if (wsman_send_request(cl, request)) {
			ws_xml_destroy_doc(request);
			return NULL;
		}

		response = wsmc_build_envelope_from_response(cl);
		ws_xml_destroy_doc(request);
	} else {
		error("No enumeration context ???");
		return NULL;
	}

	node = ws_xml_get_child(ws_xml_get_soap_body(response),
			0, NULL, NULL);

	if (node == NULL ||
			(strcmp(ws_xml_get_node_local_name(node), WSENUM_PULL_RESP)) != 0) {
		error("no Pull response");
	}
	return response;
}



WsXmlDocH
wsmc_action_release(WsManClient * cl,
		const char *resource_uri,
		client_opt_t *options,
		const char *enumContext)
{
	WsXmlDocH       response;

	if (enumContext || (enumContext && enumContext[0] == 0)) {
		WsXmlDocH request = wsmc_create_request(cl, resource_uri, options, NULL,
				WSMAN_ACTION_RELEASE,
				NULL, (char *)enumContext);
		if (!request)
			return NULL;
		if (wsman_send_request(cl, request)) {
			ws_xml_destroy_doc(request);
			return NULL;
		}
		response = wsmc_build_envelope_from_response(cl);
		ws_xml_destroy_doc(request);
	} else {
		return NULL;
	}
	return response;
}

WsXmlDocH wsmc_action_subscribe(WsManClient * cl, const char *resource_uri,
				 client_opt_t * options, filter_t *filter)
{
	WsXmlDocH response;
	WsXmlDocH request = wsmc_create_request(cl, resource_uri, options, filter,
			WSMAN_ACTION_SUBSCRIBE, NULL, NULL);
	if (!request)
		return NULL;
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}

WsXmlDocH wsmc_action_unsubscribe(WsManClient * cl, const char *resource_uri,
				 client_opt_t * options,
				 const char *subsContext)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsmc_create_request(cl, resource_uri, options, NULL,
			WSMAN_ACTION_UNSUBSCRIBE, NULL, (void *)subsContext);
	if (!request)
		return NULL;
	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}

WsXmlDocH wsmc_action_renew(WsManClient * cl, const char *resource_uri,
				 client_opt_t * options,
				 const char *subsContext)
{
	WsXmlDocH       response;
	WsXmlDocH       request = wsmc_create_request(cl, resource_uri, options, NULL,
			WSMAN_ACTION_RENEW, NULL, (void *)subsContext);
	if (!request)
		return NULL;

	if (wsman_send_request(cl, request)) {
		ws_xml_destroy_doc(request);
		return NULL;
	}
	response = wsmc_build_envelope_from_response(cl);
	ws_xml_destroy_doc(request);
	return response;
}

char*
wsmc_get_enum_context(WsXmlDocH doc)
{
	char           *enumContext = NULL;
	char           *text = NULL;
	WsXmlNodeH      enumStartNode = ws_xml_get_child(ws_xml_get_soap_body(doc),
			0, NULL, NULL);

	if (enumStartNode) {
		WsXmlNodeH      cntxNode = ws_xml_get_child(enumStartNode, 0,
				XML_NS_ENUMERATION,
				WSENUM_ENUMERATION_CONTEXT);
		if (!cntxNode) {
			return NULL;
		}
		text = ws_xml_get_node_text(cntxNode);
		if (text) {
			enumContext = u_strdup(text);
		}
	}
	return enumContext;
}

char *
wsmc_get_event_enum_context(WsXmlDocH doc)
{
	char           *enumContext = NULL;
	char           *text = NULL;
	WsXmlNodeH      node = ws_xml_get_child(ws_xml_get_soap_body(doc),
			0, XML_NS_EVENTING, WSEVENT_SUBSCRIBE_RESP);

	if (node) {
		node = ws_xml_get_child(node, 0,
				XML_NS_ENUMERATION,
				WSENUM_ENUMERATION_CONTEXT);
		if (!node)
			return NULL;
		text = ws_xml_get_node_text(node);
		if (text) {
			enumContext = u_strdup(text);
		}
	}
	return enumContext;
}

void
wsmc_free_enum_context(char *enumcontext)
{
	u_free(enumcontext);
}


/**
 * Build Inbound Envelope from Response
 * @param cl Client Handler
 * @return XML document with Envelope
 */

WsXmlDocH
wsmc_build_envelope_from_response(WsManClient * cl)
{
	WsXmlDocH       doc = NULL;
	u_buf_t        *buffer = cl->connection->response;

	if (!buffer || !u_buf_ptr(buffer)) {
		error("NULL response");
		return NULL;
	}
	doc = ws_xml_read_memory( u_buf_ptr(buffer), u_buf_len(buffer), cl->content_encoding, 0);
	if (doc == NULL) {
		error("could not create xmldoc from response");
	}
	return doc;
}


static void
wsmc_release_conn(WsManConnection * conn)
{
	if (conn == NULL) {
		return;
	}
	if (conn->request) {
		u_buf_free(conn->request);
		conn->request = NULL;
	}
	if (conn->response) {
		u_buf_free(conn->response);
		conn->response = NULL;
	}
	u_free(conn);
}



void
wsmc_reinit_conn(WsManClient * cl)
{
	u_buf_clear(cl->connection->response);
	u_buf_clear(cl->connection->request);
	cl->response_code = 0;
	cl->last_error = 0;
	if (cl->fault_string) {
		u_free(cl->fault_string);
		cl->fault_string = NULL;
	}
}


static void
init_client_connection(WsManClient * cl)
{
	WsManConnection *conn = (WsManConnection *) u_zalloc(sizeof(WsManConnection));
	u_buf_create(&conn->response);
	u_buf_create(&conn->request);
	cl->response_code = 0;
	cl->connection = conn;
}


WsManClient*
wsmc_create_from_uri(const char* endpoint)
{
	u_uri_t *uri = NULL;
	WsManClient* cl;
	if (!endpoint) {
		return NULL;
	}
	if (u_uri_parse((const char *) endpoint, &uri) != 0 ) {
		return NULL;
	}
	if (!uri) {
		return NULL;
	}
	cl = wsmc_create( uri->host,
			uri->port,
			uri->path,
			uri->scheme,
			uri->user,
			uri->pwd);
	u_uri_free(uri);
	return cl;
}

int
wsmc_set_encoding(WsManClient *cl, const char *encoding)
{
	u_free(cl->content_encoding);
	cl->content_encoding = encoding ? u_strdup(encoding) : NULL;
	return 0;
}

int
wsmc_set_namespace(WsManClient *cl, const char *ns)
{
	u_free(cl->cim_ns);
	cl->cim_ns = ns ? u_strdup(ns) : NULL;
	return 0;
}

char *wsmc_get_namespace(WsManClient *cl)
{
	return cl->cim_ns;
}

int
wsmc_check_for_fault(WsXmlDocH doc )
{
	return wsman_is_fault_envelope(doc);
}


WsManFault *
wsmc_fault_new(void)
{
	WsManFault     *fault =
		(WsManFault *) u_zalloc(sizeof(WsManFault));

	if (fault)
		return fault;
	else
		return NULL;
}

void wsmc_fault_destroy(WsManFault *fault)
{
	/*
	if (fault->code)
		u_free(fault->code);
	if (fault->subcode)
		u_free(fault->subcode);
	if (fault->reason)
		u_free(fault->reason);
	if (fault->fault_detail)
		u_free(fault->fault_detail);
		*/
	u_free(fault);
	return;
}



void
wsmc_get_fault_data(WsXmlDocH doc,
		WsManFault *fault)
{
	WsXmlNodeH body;
	WsXmlNodeH fault_node;
	WsXmlNodeH code;
	WsXmlNodeH reason;
	WsXmlNodeH detail;
	if (wsmc_check_for_fault(doc) == 0 || !fault )
		return;

	body = ws_xml_get_soap_body(doc);
	fault_node = ws_xml_get_child(body, 0, XML_NS_SOAP_1_2, SOAP_FAULT);
	if (!fault_node)
		return;

	code = ws_xml_get_child(fault_node, 0, XML_NS_SOAP_1_2, SOAP_CODE);
	if (code) {
		WsXmlNodeH code_v = ws_xml_get_child(code, 0 , XML_NS_SOAP_1_2, SOAP_VALUE);
		WsXmlNodeH subcode = ws_xml_get_child(code, 0 , XML_NS_SOAP_1_2, SOAP_SUBCODE);
		WsXmlNodeH subcode_v = ws_xml_get_child(subcode, 0 , XML_NS_SOAP_1_2, SOAP_VALUE);
		fault->code = ws_xml_get_node_text(code_v);
		fault->subcode = ws_xml_get_node_text(subcode_v);
	}
	reason = ws_xml_get_child(fault_node, 0, XML_NS_SOAP_1_2, SOAP_REASON);
	if (reason) {
		WsXmlNodeH reason_text = ws_xml_get_child(reason, 0 , XML_NS_SOAP_1_2, SOAP_TEXT);
		fault->reason = ws_xml_get_node_text(reason_text);
	}
	detail = ws_xml_get_child(fault_node, 0, XML_NS_SOAP_1_2, SOAP_DETAIL);
	if (detail) {
		// FIXME
		// WsXmlNodeH fault_detail = ws_xml_get_child(detail, 0 , XML_NS_WS_MAN, SOAP_FAULT_DETAIL);
		// fault->fault_detail = ws_xml_get_node_text(fault_detail);
		fault->fault_detail = ws_xml_get_node_text(detail);
	}
	return;
}



WsManClient*
wsmc_create(const char *hostname,
		const int port,
		const char *path,
		const char *scheme,
		const char *username,
		const char *password)
{
#ifndef _WIN32
        dictionary *ini;
#endif
	WsManClient *wsc = (WsManClient *) calloc(1, sizeof(WsManClient));
        if (wsc == NULL) {
          error("Can't alloc WsManClient");
          return NULL;
        }
	wsc->hdl = &wsc->data;

	if (pthread_mutex_init(&wsc->mutex, NULL)) {
		u_free(wsc);
		return NULL;
	}
#ifndef _WIN32
	wsmc_set_conffile(wsc, DEFAULT_CLIENT_CONFIG_FILE);
        ini = iniparser_new(wsmc_get_conffile(wsc));
        if (ini) {
          char *user_agent = iniparser_getstr(ini, "client:agent");
          if (user_agent) {
            wsman_transport_set_agent(wsc, user_agent);
          }
          iniparser_free(ini);
        }
#endif
	wsc->serctx = ws_serializer_init();
	wsc->dumpfile = stdout;
	wsc->data.scheme = u_strdup(scheme ? scheme : "http");
	wsc->data.hostname = hostname ? u_strdup(hostname) : u_strdup("localhost");
	wsc->data.port = port;
	wsc->data.path = u_strdup(path ? path : "/wsman");
	wsc->data.user = username ? u_strdup(username) : NULL;
	wsc->data.pwd = password ? u_strdup(password) : NULL;
	wsc->data.auth_set = 0;
	wsc->initialized = 0;
	wsc->transport_timeout = 0;
	wsc->content_encoding = u_strdup("UTF-8");
#ifdef _WIN32
	wsc->session_handle = 0;
#endif
	wsc->data.endpoint = u_strdup_printf("%s://%s:%d%s%s",
                                             wsc->data.scheme, wsc->data.hostname,
                                             wsc->data.port,
                                             (*wsc->data.path == '/') ? "" : "/",
                                             wsc->data.path);
	debug("Endpoint: %s", wsc->data.endpoint);
	wsc->authentication.verify_host = 1; //verify CN in server certificates by default
	wsc->authentication.verify_peer = 1; //validate server certificates by default
#ifndef _WIN32
	wsc->authentication.crl_check = 0;   // No CRL check by default
	wsc->authentication.crl_file = NULL;
#endif

	init_client_connection(wsc);

	return wsc;
}



void
wsmc_release(WsManClient * cl)
{
#ifndef _WIN32
	if (cl->client_config_file) {
		u_free(cl->client_config_file);
		cl->client_config_file = NULL;
	}
#endif
	if (cl->data.scheme) {
		u_free(cl->data.scheme);
		cl->data.scheme = NULL;
	}
	if (cl->data.hostname) {
		u_free(cl->data.hostname);
		cl->data.hostname = NULL;
	}
	if (cl->data.path) {
		u_free(cl->data.path);
		cl->data.path = NULL;
	}
	if (cl->data.user) {
		u_free(cl->data.user);
		cl->data.user = NULL;
	}

	if (cl->data.pwd) {
		u_cleanfree(cl->data.pwd);
		cl->data.pwd = NULL;
	}

	if (cl->data.endpoint) {
		u_free(cl->data.endpoint);
		cl->data.endpoint = NULL;
	}
	if (cl->fault_string) {
		u_free(cl->fault_string);
		cl->fault_string = NULL;
	}
	if (cl->connection) {
		wsmc_release_conn(cl->connection);
		cl->connection = NULL;
	}
	if (cl->serctx) {
		ws_serializer_cleanup(cl->serctx);
		cl->serctx = NULL;
	}
	if (cl->content_encoding) {
		u_free(cl->content_encoding);
		cl->content_encoding = NULL;
	}
	if (cl->cim_ns) {
		u_free(cl->cim_ns);
		cl->cim_ns = NULL;
	}
	if (cl->authentication.crl_file != NULL) {
		u_free(cl->authentication.crl_file);
		cl->authentication.crl_file = NULL;
	}
	if (cl->authentication.method != NULL) {
		u_free(cl->authentication.method);
		cl->authentication.method = NULL;
	}
        if (cl->authentication.cainfo != NULL) {	 
          u_free(cl->authentication.cainfo);
          cl->authentication.cainfo = NULL;
        }
        if (cl->authentication.caoid != NULL) {
          u_free(cl->authentication.caoid);
          cl->authentication.caoid = NULL;
        }
        if (cl->proxy_data.proxy_username != NULL) {
          u_free(cl->proxy_data.proxy_username);
          cl->proxy_data.proxy_username = NULL;
        }
        if (cl->proxy_data.proxy_password != NULL) {
          u_cleanfree(cl->proxy_data.proxy_password);
          cl->proxy_data.proxy_password = NULL;
        }
        pthread_mutex_destroy(&cl->mutex);

	wsman_transport_close_transport(cl);

	u_free(cl);
}

int
wsmc_lock(WsManClient * cl)
{
	if (pthread_mutex_lock(&cl->mutex))
		return 1;

	if (cl->flags & WSMAN_CLIENT_BUSY) {
		pthread_mutex_unlock(&cl->mutex);
		return 1;
	}
	cl->flags |= WSMAN_CLIENT_BUSY;
	pthread_mutex_unlock(&cl->mutex);
	return 0;
}


void
wsmc_unlock(WsManClient * cl)
{
	if (pthread_mutex_lock(&cl->mutex))
		return;

	cl->flags &= ~WSMAN_CLIENT_BUSY;
	pthread_mutex_unlock(&cl->mutex);
}


void
wsmc_remove_query_string(const char *s, char **result)
{
	char           *r = 0;
	const char     *q;
	char           *buf = 0;

	buf = u_strndup(s, strlen(s));
	if (!buf) {
		*result = NULL;
		return;
	}
	if ((q = strchr(buf, '?')) != NULL) {
		r = u_strndup(s, q - buf);
		*result = r;
	} else {
		*result = (char *)s;
	}

	u_free(buf);
}
