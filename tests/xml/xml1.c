

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "u/libu.h"


#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-errors.h"
#include "wsman-soap.h"
#include "wsman-xml-serialize.h"


struct __CIM_ComputerSystem {
    char *NameFormat;
    char *test[2];
    XmlSerialiseDynamicSizeData foo;
};
typedef struct __CIM_ComputerSystem CIM_ComputerSystem;
/*
struct __XmlSerializerInfo foo_TypeInfo[] =
{
      SER_STR("foo", 1, 1)
};
*/

SER_TYPEINFO_STRING;

SER_START_ITEMS("CIM_ComputerSystem", CIM_ComputerSystem)
SER_STR("NameFormat", 1),
SER_STR("test", 2),
SER_DYN_ARRAY("foo", string),
SER_END_ITEMS("CIM_ComputerSystem", CIM_ComputerSystem);


#define NS "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem"

int main(void)
{
    WsContextH cntx = ws_create_runtime(NULL);
    SoapH soap = ws_context_get_runtime(cntx);

    WsXmlDocH doc = ws_xml_read_file(soap, "cim_computersystem_01.xml",
                                     "UTF-8", 0 ); 
    WsXmlNodeH node = ws_xml_get_soap_body(doc);
    CIM_ComputerSystem *cs  = ws_deserialize(cntx,
                                     node,
                                     CIM_ComputerSystem_TypeInfo,
                                     "CIM_ComputerSystem",
                                     NS, NS,
                                     0, 0);

    if (cs == NULL) {
        printf("No cs\n");
        return 1;
    }
    printf("NameFormat = <%s>\n", cs->NameFormat);
    printf("test[0] <%s>\n", cs->test[0]);
    printf("test[1] <%s>\n", cs->test[1]);

    printf("cs->foo.count = %d\n", cs->foo.count);
    if (cs->foo.data == NULL) {
        printf("No cs->foo.data\n");
        return 1;
    }
    int i;
    char **p = (char **)cs->foo.data;
    for (i = 0; i < cs->foo.count; i++) {
        printf("      foo[%d] = <%s>\n", i, *p);
        p++;
    }
    return 0;
}
