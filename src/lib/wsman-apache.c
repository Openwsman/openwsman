/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
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
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Vadim Revyakin
 */
#include "wsman-types.h"
#include "wsman-apache.h"
#include "wsman-server.h"
#include "wsman-dispatcher.h"
#include "wsman-soap.h"



void *
wsman_apache_create_config(void)
{
    SoapH soap = NULL;
    WsManListenerH *listener = wsman_dispatch_list_new();
    WsContextH cntx = wsman_init_plugins(listener);
    if (cntx != NULL) {
        soap = ws_context_get_runtime(cntx);
    }
    return (void *)soap;
}


u_buf_t *
wsman_apache_get_response(void *arg, char *request)
{
    WsmanMessage *wsman_msg;
    u_buf_t  *u_buf;
    SoapH  soap = (SoapH)arg;

    // XXX. check POST method

    wsman_msg = wsman_soap_message_new();
    wsman_msg->request.body = request;
    wsman_msg->request.length = strlen(request);

        // Call dispatcher
    dispatch_inbound_call(soap, wsman_msg);

    u_buf_create(&u_buf);
    u_buf_set(u_buf, wsman_msg->response.body,
                        wsman_msg->response.length);

    wsman_msg->request.body = NULL;
    wsman_soap_message_destroy(wsman_msg);

    return u_buf;
}





