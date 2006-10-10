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
 * @author Anas Nashif
 */

#include "wsman_config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "u/libu.h"
#include "wsman-xml-api.h"
#include "wsman-soap.h"
#include "wsman-dispatcher.h"
#include "wsman-soap-envelope.h"

#include "wsman-xml.h"
#include "wsman-xml-serializer.h" 
#include "wsman-faults.h"
#include "wsman-plugins.h"
#include "wsman-server.h"



WsManListenerH*
wsman_dispatch_list_new()
{
    WsManListenerH *list = (WsManListenerH *)u_malloc(sizeof(WsManListenerH) );
    return list;
}

WsContextH
wsman_init_plugins(WsManListenerH *listener)
{	
    list_t *list = list_create(LISTCOUNT_T_MAX);
    WsContextH cntx = NULL;	  	
    wsman_plugins_load(listener);
    lnode_t *node = list_first(listener->plugins);

    while (node) 
    {		
        WsManPlugin *p = (WsManPlugin *)node->list_data;
        p->interface = (WsDispatchInterfaceInfo *)malloc(sizeof(WsDispatchInterfaceInfo));

        p->get_endpoints = dlsym(p->p_handle, "get_endpoints");
        p->set_config = dlsym(p->p_handle, "set_config");
        p->get_endpoints(p->p_handle, p->interface );
        if (listener->config)
            p->set_config(p->p_handle, listener->config);

        lnode_t *i = lnode_create(p->interface);
        list_append(list, i);
        node = list_next(listener->plugins, node );
    }
    cntx = ws_create_runtime(list);
    return cntx;
}

