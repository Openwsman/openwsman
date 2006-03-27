

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>

#include "ws_utilities.h"


#include "ws_xml_api.h"
#include "ws_errors.h"
#include "soap_api.h"
#include "xml_api_generic.h"
#include "xml_serializer.h"


int main(void)
{
    
    g_type_init ();
    g_thread_init (NULL);
    WsContextH cntx = ws_create_runtime(NULL);

    SOAP_FW* soap = (SOAP_FW*)ws_context_get_runtime(cntx);
    WsXmlDocH doc = ws_xml_read_file (soap, "./data/xml_test1.xml", NULL, 0);
    if (doc) {
         ws_xml_dump_node_tree(stdout, ws_xml_get_doc_root(doc), 1);
    }
    
    char* nameNsPrefix = ws_xml_get_node_name_ns_prefix(ws_xml_get_child(ws_xml_get_soap_body(doc), 0 , NULL, NULL));
    char* Uri = ws_xml_get_node_name_ns_uri(ws_xml_get_child(ws_xml_get_soap_body(doc), 0 , NULL, NULL));
    fprintf(stdout, "\nnamespace: %s\n",  nameNsPrefix);
    fprintf(stdout, "namespace uri: %s\n",  Uri);
    
    ws_xml_parser_destroy((SoapH)cntx);
    soap_free(cntx);
    return 0;
}
