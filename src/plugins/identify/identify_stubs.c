
#include "config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "gmodule.h"


#include "wsman-util.h"

#include "wsman-errors.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml-serializer.h"
#include "wsman-dispatcher.h"

#include "identify.h"


wsmid_identify g_wsmid_identify = { XML_NS_WS_MAN, "Openwsman Project" ,  PACKAGE_VERSION };


wsmid_identify* wsmid_identify_Identify_EP(WsContextH cntx)
{

    return &g_wsmid_identify;
}

