/*******************************************************************************
* Copyright (C) 2004-2007 Intel Corp. All rights reserved.
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
 * @author Liang Hou
 */
 
#ifndef WSMAN_EVENT_SOURCE_H_
#define WSMAN_EVENT_SOURCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "u/list.h"
  
#define EUIDLEN		64

struct _WsXmlDoc;


struct __WsNotificationInfo {
	struct _WsXmlDoc *headerOpaqueData; //header content
	char *EventAction; //event action URL
	struct _WsXmlDoc *EventContent; //event content
};
typedef struct __WsNotificationInfo * WsNotificationInfoH;

/*to store events for a subscription*/
struct __event_entry {
   char subscription_id[EUIDLEN]; //to identify the event entry
   list_t *event_content_list; //a list holds __WsNotificationInfo
};
typedef struct __event_entry *event_entryH;

typedef void (*clearproc) (WsNotificationInfoH);

typedef int (*EventPoolInit) (void *);
typedef int (*EventPoolFinalize) (void *);
typedef int (*EventPoolCount) (char *);
typedef int (*EventPoolAddEvent) (char *, WsNotificationInfoH);
typedef int (*EventPoolAddPullEvent) (char *, WsNotificationInfoH);
typedef int (*EventPoolGetAndDeleteEvent) (char *, WsNotificationInfoH*);
typedef int (*EventPoolClearEvent) (char *, clearproc);

/*Event Source Function Table*/
struct __EventPoolOpSet {
	EventPoolInit init;
	EventPoolFinalize finalize;
	EventPoolCount count;
	EventPoolAddEvent add;
	EventPoolAddPullEvent addpull;
	EventPoolGetAndDeleteEvent remove;
	EventPoolClearEvent clear;
};
typedef struct __EventPoolOpSet *EventPoolOpSetH;

EventPoolOpSetH wsman_get_eventpool_opset(void);

#ifdef __cplusplus
}
#endif
    
#endif
