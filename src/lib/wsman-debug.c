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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "wsman-util.h"
#include "wsman-debug.h"
#include <assert.h>


struct _WsmanDebugHandler {
    WsmanDebugFn    	fn;
    WsmanDebugLevel 	level;
    void     		*user_data;
    unsigned int       	id;
};
typedef struct _WsmanDebugHandler WsmanDebugHandler;

static DL_List  handlers;

unsigned int
wsman_debug_add_handler (
        WsmanDebugFn    fn,
        WsmanDebugLevel level,
        void      *user_data)
{
    WsmanDebugHandler *handler;
    assert (fn);
    handler = (WsmanDebugHandler *) soap_alloc (sizeof(WsmanDebugHandler), 1);

    handler->fn = fn;
    handler->level = level;
    handler->user_data = user_data;

    if (DL_GetCount(&handlers) > 0) {
        handler->id = DL_GetCount(&handlers) + 1;
    } else {
        handler->id = 1;
    }
    DL_MakeNode(&handlers, handler);
    printf("id: %d\n", handler->id );
    return handler->id;
}

void
wsman_debug_remove_handler (unsigned int id)
{
    DL_Node *iter;
    iter = DL_GetHead(&handlers);
    while (iter) {
        WsmanDebugHandler *handler = (WsmanDebugHandler *)iter->dataBuf;

        if (handler->id == id) {
            iter =  DL_RemoveNode ( iter);
            free (handler);
            return;
        }
        iter = iter->next;
    }

    wsman_debug (WSMAN_DEBUG_LEVEL_WARNING, "Couldn't find debug handler %d", id);
}

const char *
wsman_debug_helper (
        const char *format,
                 ...)
{
    va_list args;
    static char *str = NULL;
    size_t const initialSize = 4096;

    str = soap_alloc(initialSize, 1 );
    va_start (args, format);
    vsprintf (str, format, args);
    va_end (args);

    return str;
}

void
wsman_debug_full (WsmanDebugLevel  level,
               const char   *format,
               ...)
{
    va_list args;
    DL_Node *iter;
    char *str;
    size_t const initialSize = 4096;
    str = soap_alloc(initialSize, 1 );

    va_start (args, format);
    vsprintf (str, format, args);
    va_end (args);

    iter = DL_GetHead(&handlers);
    while (iter) {
        WsmanDebugHandler *handler = (WsmanDebugHandler *)iter->dataBuf;

        if ((handler->level == WSMAN_DEBUG_LEVEL_ALWAYS) ||
            (level <= handler->level))
            handler->fn (str, level, handler->user_data);

        iter = iter->next;
    }

    free (str);
}
