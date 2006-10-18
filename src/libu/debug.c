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

#include "u/libu.h"


static list_t *handlers = NULL;

unsigned int
debug_add_handler (debug_fn    fn,
                   debug_level_e level,
                   void*     user_data)
{

    debug_handler_t *handler = (debug_handler_t *)u_malloc(sizeof(debug_handler_t));
    if (!handlers)
        handlers = list_create(LISTCOUNT_T_MAX);

    handler->fn = fn;
    handler->level = level;
    handler->user_data = user_data;

    if (list_count(handlers) > 0 ) {
        lnode_t *n = list_last(handlers);
        handler->id = ((debug_handler_t *)n->list_data)->id  + 1;
    }
    else
        handler->id = 1;

    lnode_t *new = lnode_create(handler);
    list_append(handlers, new);

    return handler->id;
}

void
debug_remove_handler (unsigned int id)
{
    lnode_t *iter = list_first(handlers);
    while (iter) {
        debug_handler_t *handler = (debug_handler_t *)iter->list_data;

        if (handler->id == id) {
            list_delete (handlers, iter);
            lnode_destroy(iter);
            return;
        }
        iter = list_next(handlers, iter);
    }
}

const char *
debug_helper (const char *format,
                 ...)
{
    va_list args;
    static char *str = NULL;

    /*
    if (str)
        u_free (str);
        */

    va_start (args, format);
    str = u_strdup_vprintf (format, args);
    va_end (args);
    return str;
}

void
debug_full (debug_level_e  level,
            const char   *format,
               ...)
{
    va_list args;
    char *str;
    
    if (handlers == NULL) {
        return;
    }
    
    va_start (args, format);
    str = u_strdup_vprintf (format, args);
    va_end (args);

    lnode_t *iter = list_first(handlers);
    while (iter) {
        debug_handler_t *handler = (debug_handler_t *)iter->list_data;

        if ((handler->level == DEBUG_LEVEL_ALWAYS) ||
            (level <= handler->level))
            handler->fn (str, level, handler->user_data);

        iter = list_next(handlers, iter);
    }
    u_free (str);
}
