
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
struct __WsManTest2
{
	char* Test;
};
typedef struct __WsManTest2 WsManTest2;


struct __XmlSerializerInfo WsManTest2_TypeItems[] =
{
        SER_STR("Test", 1, 1),
        SER_LAST
};

struct __XmlSerializerInfo WsManTest2_TypeInfo[] =
{
        SER_STRUCT("Test2", 1, 1, WsManTest2)
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
    
    GList *prop;
    WsXmlDocH xmlDoc = cl->ft->put(cl, "wsman:system/2005/06/test2", prop);

    WsXmlNodeH soapBody = ws_xml_get_soap_body(xmlDoc);
    if (ws_xml_get_child(soapBody, 0, XML_NS_WS_MAN"/test2", "Test2")) {
        WsManTest2 *test2 = ws_deserialize(
                cntx, 
                soapBody,     
                WsManTest2_TypeInfo,
                "Test2",
                XML_NS_WS_MAN"/test2",
                XML_NS_WS_MAN"/test2",
                0,
                0);
        printf(" Test: %s\n", test2->Test);
    }
    

    
    ws_xml_destroy_doc(xmlDoc);
    soap_destroy_fw(ws_context_get_runtime(cntx));
    cl->ft->release(cl);

    return (0);
}

