
/* verify that ws_serializer_free_mem works */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wsman-api.h>
#include <wsman-xml.h>

struct __CIM_ComputerSystem {
    unsigned int number;
    char *NameFormat;
    char *test[2];
    XmlSerialiseDynamicSizeData foo;
};
typedef struct __CIM_ComputerSystem CIM_ComputerSystem;

#define NS "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"

SER_TYPEINFO_STRING;

SER_START_ITEMS(CIM_ComputerSystem)
SER_NS_UINT32(NS, "number", 1),
SER_NS_STR(NS, "NameFormat", 1),
SER_NS_STR(NS, "test", 2),
SER_NS_DYN_ARRAY(NS, "foo", 1, 100, string),
SER_END_ITEMS(CIM_ComputerSystem);

static void
debug_handler(const char *message, debug_level_e level, void *user_data)
{
    fprintf(stderr, ">>> %s\n", message);
}

int
main(int argc, char *argv[])
{
    debug_add_handler(debug_handler, DEBUG_LEVEL_WARNING, NULL);

    if (argc < 2) {
        fprintf(stderr, "no filename\n");
        return 1;
    }

    WsSerializerContextH cntx = ws_serializer_init();
    WsXmlDocH doc = ws_xml_read_file(argv[1], "UTF-8", 0);
    WsXmlNodeH node = ws_xml_get_soap_body(doc);

    if (!node) {
        fprintf(stderr, "no xml\n");
        return 1;
    }

    CIM_ComputerSystem *cs = ws_deserialize(cntx, node,
                                            CIM_ComputerSystem_TypeInfo,
                                            "CIM_ComputerSystem",
                                            NULL, NULL, 0, 0);

    if (cs == NULL) {
        fprintf(stderr, "no cs\n");
        return 1;
    }

    if (ws_serializer_free_mem(cntx, cs, CIM_ComputerSystem_TypeInfo) < 0) {
        fprintf(stderr, "ws_serializer_free_mem failed\n");
        return 1;
    }

    return 0;
}
