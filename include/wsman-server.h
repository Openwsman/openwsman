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
 * @author Liang Hou
 */

#ifndef WSMAN_SERVER_H_
#define WSMAN_SERVER_H_
#include "wsman-soap.h"
#include "wsman-plugins.h"
#include "wsman-subscription-repository.h"
#include "wsman-event-pool.h"
int continue_working;

WsContextH wsman_init_plugins(WsManListenerH *listener);
SubsRepositoryOpSetH wsman_init_subscription_repository(WsContextH cntx, char *uri);
EventPoolOpSetH wsman_init_event_pool(WsContextH cntx, void*data);
WsManListenerH *wsman_dispatch_list_new(void);
void *wsman_server_auxiliary_loop_thread(void *arg);
int wsman_clean_subsrepository(SoapH soap, SubsRepositoryEntryH entry);
void wsman_repos_notification_dispatcher(WsContextH cntx, SubsRepositoryEntryH entry, int subsNum);
void *wsman_notification_manager(void *arg);

#endif
