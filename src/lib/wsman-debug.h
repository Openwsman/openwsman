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

#ifndef WSMANDEBUG_H_
#define WSMANDEBUG_H_

#include "config.h"
#include <glib.h>

#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    WSMAN_DEBUG_LEVEL_ALWAYS   = -1,
    WSMAN_DEBUG_LEVEL_NONE     = 0,
    WSMAN_DEBUG_LEVEL_ERROR    = 1,
    WSMAN_DEBUG_LEVEL_CRITICAL = 2,
    WSMAN_DEBUG_LEVEL_WARNING  = 3,
    WSMAN_DEBUG_LEVEL_MESSAGE  = 4,
    WSMAN_DEBUG_LEVEL_INFO     = 5,
    WSMAN_DEBUG_LEVEL_DEBUG    = 6,
} WsmanDebugLevel;

typedef void (*WsmanDebugFn) (const char *message,
                           WsmanDebugLevel level,
                           gpointer user_data);

guint
wsman_debug_add_handler (WsmanDebugFn fn,
                      WsmanDebugLevel level,
                      gpointer user_data);

void
wsman_debug_remove_handler (guint id);

const char *
wsman_debug_helper (const char *format, ...);

void
wsman_debug_full (WsmanDebugLevel  level,
               const char   *format,
               ...);

#ifdef WSMAN_DEBUG_VERBOSE

const char *
wsman_debug_helper (const char *format,
                 ...);

#define wsman_debug(level, format...) \
        wsman_debug_full(level, "[%d] %s:%d(%s) %s", level, __FILE__, __LINE__,__FUNCTION__, \
                wsman_debug_helper (format))
#else

#define wsman_debug wsman_debug_full

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*WSMANDEBUG_H_*/
