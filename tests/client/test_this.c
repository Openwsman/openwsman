
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

#include "wsman-client.h"


#include "wsman.h"

// The resource is modeled as a struct
struct __WsManThis
{
	char* Vendor;
	char* Version;
};
typedef struct __WsManThis WsManThis;


struct __XmlSerializerInfo WsManThis_TypeItems[] =
{
        SER_STR("Vendor", 1, 1),
        SER_STR("Version", 1, 1),
        SER_LAST
};

struct __XmlSerializerInfo WsManThis_TypeInfo[] =
{
        SER_STRUCT("This", 1, 1, WsManThis)
};

int main(void) {
    g_type_init ();
    g_thread_init (NULL);

    WsContextH cntx = ws_create_runtime(NULL);
    
    WsManClient *cl = 
            wsman_connect(
                cntx,
                "localhost",
                8889,
                NULL,
                "wsman",
                "secret",
                NULL);
                
                
    WsManClientEnc *wsc =(WsManClientEnc*)cl;
    if (cl == NULL)
    {
            fprintf(stderr, "Null Client\n");
    } 
    wsman_client_add_handler(wsman_client_handler, NULL);
    WsXmlDocH xmlDoc = cl->ft->get(cl, "wsman:system/2005/06/this");
    WsXmlNodeH soapBody = ws_xml_get_soap_body(xmlDoc);
    if (ws_xml_get_child(soapBody, 0, XML_NS_WS_MAN"/this", "This")) {
        WsManThis *this = ws_deserialize(
                cntx, 
                soapBody,     
                WsManThis_TypeInfo,
                "This",
                XML_NS_WS_MAN"/this",
                XML_NS_WS_MAN"/this",
                0,
                0);
        printf(" Vendor: %s\n", this->Vendor);
        printf(" Version: %s\n", this->Version);
    }
    

    
    ws_xml_destroy_doc(xmlDoc);
    soap_destroy_fw(ws_context_get_runtime(cntx));
    cl->ft->release(cl);

    return (0);
}

