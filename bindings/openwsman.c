

#include "wsman-api.h"
#include "openwsman.h"

char *_identify(WsManClient * cl, client_opt_t * options, char *encoding) {
	char *buf = NULL;
	int len;
	WsXmlDocH doc = wsmc_action_identify(cl, options);
	ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
	ws_xml_destroy_doc(doc);
	return buf;
}
char *_enumerate(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding) {
	char *buf = NULL;
	int len;
	WsXmlDocH doc  = wsmc_action_enumerate(cl, resource_uri,options);
	ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
	ws_xml_destroy_doc(doc);
	return buf;
}
char *_pull(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *enumContext, char *encoding) {
	char *buf = NULL;
	int len;
	WsXmlDocH doc  = wsmc_action_pull(cl, resource_uri,options, enumContext);
	ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
	ws_xml_destroy_doc(doc);
	return buf;
}
char *_release(WsManClient * cl, const char *resource_uri, client_opt_t * options, const char *enumContext, char *encoding) {
	char *buf = NULL;
	int len;
	WsXmlDocH doc  = wsmc_action_release(cl, resource_uri,options, enumContext);
	ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
	ws_xml_destroy_doc(doc);
	return buf;
}
char *_get(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding) {
	char *buf = NULL;
	int len;
	WsXmlDocH doc  = wsmc_action_get(cl, resource_uri, options);
	ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
	ws_xml_destroy_doc(doc);
	return buf;
}
char *_delete(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding) {
	char *buf = NULL;
	int len;
	WsXmlDocH doc  = wsmc_action_delete(cl, resource_uri, options);
	ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
	ws_xml_destroy_doc(doc);
	return buf;
}

char *_invoke(WsManClient * cl, const char *resource_uri, client_opt_t * options,
                const char *method, const char *data, size_t size, char *encoding) {
	char *buf = NULL;
	int len;
	WsXmlDocH doc  = wsmc_action_invoke_fromtext(cl, resource_uri, options, method, data, size, encoding);
	ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
	ws_xml_destroy_doc(doc);
	return buf;
}
char *_put(WsManClient * cl, const char *resource_uri, client_opt_t * options,
                const char *data, size_t size, char *encoding) {
	char *buf = NULL;
	int len;
	WsXmlDocH doc  = wsmc_action_put_fromtext(cl, resource_uri, options, data, size, encoding);
	ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
	ws_xml_destroy_doc(doc);
	return buf;
}

char *_subscribe(WsManClient * cl, const char *resource_uri, client_opt_t * options, char *encoding) {
	char *buf = NULL;
        int len;
        WsXmlDocH doc  = wsmc_action_subscribe(cl, resource_uri, options);
        ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
        ws_xml_destroy_doc(doc);
        return buf;
}

char *_renew(WsManClient *cl, const char *resource_uri, client_opt_t *options, char *identifier, char *encoding) {
	char *buf = NULL;
        int len;
        WsXmlDocH doc  = wsmc_action_renew(cl, resource_uri, options, identifier);
        ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
        ws_xml_destroy_doc(doc);
        return buf;
}

char *_unsubscribe(WsManClient *cl, const char *resource_uri, client_opt_t *options, char *identifier, char *encoding) {
	char *buf = NULL;
        int len;
        WsXmlDocH doc  = wsmc_action_unsubscribe(cl, resource_uri, options, identifier);
        ws_xml_dump_memory_enc (doc, &buf, &len, encoding);
        ws_xml_destroy_doc(doc);
        return buf;
}

