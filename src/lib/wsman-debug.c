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

 
#include <glib.h>

#include "wsman-debug.h"

#include <stdarg.h>
#include <stdio.h>



struct _WsmanDebugHandler {
    WsmanDebugFn    	fn;
    WsmanDebugLevel 	level;
    gpointer     		user_data;
    guint        		id;
};
typedef struct _WsmanDebugHandler WsmanDebugHandler;

static GSList *handlers = NULL;

guint
wsman_debug_add_handler (WsmanDebugFn    fn,
                      WsmanDebugLevel level,
                      gpointer     user_data)
{
    WsmanDebugHandler *handler;

    g_assert (fn);

    handler = g_new0 (WsmanDebugHandler, 1);

    handler->fn = fn;
    handler->level = level;
    handler->user_data = user_data;

    if (handlers)
        handler->id = ((WsmanDebugHandler *) handlers->data)->id + 1;
    else
        handler->id = 1;

    handlers = g_slist_prepend (handlers, handler);

    return handler->id;
}

void
wsman_debug_remove_handler (guint id)
{
    GSList *iter;

    iter = handlers;
    while (iter) {
        WsmanDebugHandler *handler = (WsmanDebugHandler *)iter->data;

        if (handler->id == id) {
            handlers = g_slist_remove_link (handlers, iter);
            g_free (handler);
            return;
        }

        iter = iter->next;
    }

    wsman_debug (WSMAN_DEBUG_LEVEL_WARNING, "Couldn't find debug handler %d", id);
}

const char *
wsman_debug_helper (const char *format,
                 ...)
{
    va_list args;
    static char *str = NULL;

    if (str)
        g_free (str);

    va_start (args, format);
    str = g_strdup_vprintf (format, args);
    va_end (args);

    return str;
}

void
wsman_debug_full (WsmanDebugLevel  level,
               const char   *format,
               ...)
{
    va_list args;
    GSList *iter;
    char *str;

    va_start (args, format);
    str = g_strdup_vprintf (format, args);
    va_end (args);

    iter = handlers;
    while (iter) {
        WsmanDebugHandler *handler = (WsmanDebugHandler *)iter->data;

        if ((handler->level == WSMAN_DEBUG_LEVEL_ALWAYS) ||
            (level <= handler->level))
            handler->fn (str, level, handler->user_data);

        iter = iter->next;
    }

    g_free (str);
}
