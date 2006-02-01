
#include "config.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "ctype.h"

#include "ws_utilities.h"



#include "ws_errors.h"
#include "ws_xml_api.h"
#include "soap_api.h"
#include "xml_serializer.h"
#include "ws_dispatcher.h"

#include "wsman_this.h"




WsManThis g_WsManThis = { "Intel WS-MAN Interop", "0.1.0.0." };



// ******************* WS-MAN this *******************************

WsManThis* WsManThis_Get_EP(WsContextH cntx)
{
    return &g_WsManThis;
}

