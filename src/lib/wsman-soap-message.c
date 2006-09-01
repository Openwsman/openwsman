
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <glib.h>

#include "wsman-util.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-debug.h"
#include "wsman-faults.h"
#include "wsman-soap-envelope.h"
#include "wsman-soap-message.h"


void wsman_set_message_flags(
        WsmanMessage *msg, 
        unsigned int flag)
{
    msg->flags |= flag;
    return;
}


WsmanMessage *wsman_soap_message_new()
{
    WsmanMessage *wsman_msg = soap_alloc(sizeof(WsmanMessage), 0 );
    wsman_msg->response.body = NULL;
    wsman_msg->request.body = NULL;
    wsman_msg->request.length = 0;
    wsman_msg->response.length = 0;
    wsman_msg->status.fault_code = 0;
    wsman_msg->status.fault_detail_code = 0;
    wsman_msg->status.fault_msg = NULL;
    return wsman_msg;
};


