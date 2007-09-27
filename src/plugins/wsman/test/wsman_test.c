
#include "wsman_config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "u/libu.h"

#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"

#include "wsman_test.h"

SER_START_ITEMS(WsManTestResult)
SER_UINT8("result1", 1), 
SER_UINT8("result2", 1), 
SER_END_ITEMS(WsManTestResult);


SER_START_ITEMS(WsManTest)
SER_STR("Simple", 1), 
SER_STRUCT("Result", 1, WsManTestResult), 
SER_END_ITEMS(WsManTest);


#if 0
START_TRANSFER_GET_SELECTORS(WsManTest)
FINISH_TRANSFER_GET_SELECTORS(WsManTest);
#endif

START_END_POINTS(WsManTest)
    END_POINT_TRANSFER_GET(WsManTest, XML_NS_OPENWSMAN"/test"),
    END_POINT_ENUMERATE(WsManTest, XML_NS_OPENWSMAN"/test"),
    END_POINT_DIRECT_PULL(WsManTest, XML_NS_OPENWSMAN"/test"),
    END_POINT_PULL(WsManTest, XML_NS_OPENWSMAN"/test"),
    END_POINT_RELEASE(WsManTest, XML_NS_OPENWSMAN"/test"),
    END_POINT_TRANSFER_PUT(WsManTest, XML_NS_OPENWSMAN"/test"),
#ifdef ENABLE_EVENTING_SUPPORT
    END_POINT_SUBSCRIBE(WsManTest, XML_NS_OPENWSMAN"/test"),
    END_POINT_UNSUBSCRIBE(WsManTest, XML_NS_OPENWSMAN"/test"),
    END_POINT_RENEW(WsManTest, XML_NS_OPENWSMAN"/test"),
#endif
FINISH_END_POINTS(WsManTest);


START_NAMESPACES(WsManTest)
    ADD_NAMESPACE( XML_NS_OPENWSMAN, NULL ),
FINISH_NAMESPACES(WsManTest);

static list_t *
set_namespaces(void) 
{

  int i;

  list_t *l = list_create(LISTCOUNT_T_MAX);
  for (i = 0; WsManTest_Namespaces[i].ns != NULL; i++)
  {
    WsSupportedNamespaces *ns = (WsSupportedNamespaces *)u_malloc(sizeof(WsSupportedNamespaces));
    ns->class_prefix = WsManTest_Namespaces[i].class_prefix;
    ns->ns = (char*) WsManTest_Namespaces[i].ns;
    lnode_t *node = lnode_create(ns);
    list_append(l, node);
  }


  return l;
}


void get_endpoints(void *self, void **data) 
{		 		
    WsDispatchInterfaceInfo *ifc = 	(WsDispatchInterfaceInfo *)data;	
    ifc->flags = 0;
    ifc->actionUriBase = NULL;
    ifc->version = PACKAGE_VERSION;
    ifc->vendor = "Openwsman Project";
    ifc->displayName = "Test";
    ifc->notes = "Test Plugin";
    ifc->compliance = XML_NS_WS_MAN;
    ifc->wsmanResourceUri = WS_MAN_TEST_RESOURCE_URI;
    ifc->extraData = NULL;
    ifc->namespaces = set_namespaces();
    ifc->endPoints = WsManTest_EndPoints;			
}


int init( void *self, void **data )
{
    return 1;
}

void
cleanup( void  *self, void *data )
{
	return;
}
void set_config( void *self, dictionary *config )
{
    return;
}

