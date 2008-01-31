/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,cl
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGclE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Anas Nashif
 */

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-xml.h"
#include "wsman-xml-serializer.h"

#include "wsman-faults.h"
#include "wsman-soap-envelope.h"
#include "wsman-soap-message.h"


void 
wsman_set_message_flags(WsmanMessage *msg, 
                        unsigned int flag)
{
    msg->flags |= flag;
    return;
}


WsmanMessage*
wsman_soap_message_new()
{
    WsmanMessage *wsman_msg = u_zalloc(sizeof(WsmanMessage));
    u_buf_create(&wsman_msg->request);
    u_buf_create(&wsman_msg->response);
//    wsman_msg->charset = "UTF-8";
    wsman_msg->status.fault_code = 0;
    wsman_msg->status.fault_detail_code = 0;
    wsman_msg->status.fault_msg = NULL;
    return wsman_msg;
}

void
wsman_soap_message_destroy(WsmanMessage* wsman_msg)
{
    u_buf_free(wsman_msg->response);
    u_buf_free(wsman_msg->request);
    u_free(wsman_msg->charset);
    u_free(wsman_msg->auth_data.password);
    u_free(wsman_msg->auth_data.username);
    if (wsman_msg->status.fault_msg) {
        u_free(wsman_msg->status.fault_msg);
    }
    if (wsman_msg->http_headers) {
        hash_free(wsman_msg->http_headers);
        wsman_msg->http_headers = NULL;
    }
   u_free(wsman_msg);
}
