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

#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <u/pthreadx.h>
#include <u/lock.h>
#include <u/memory.h>


#ifndef WIN32
extern int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
#endif

#if defined (__SVR4) && defined (__sun)
#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#endif

void u_init_lock(void *data)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP );
    if ( data != NULL )
        pthread_mutex_init((pthread_mutex_t*)data, &attr);
}

int u_try_lock(void* data)
{
    int try = 0;
    if ( data )
    {
        try = pthread_mutex_trylock( (pthread_mutex_t*)data );
    }
    return try;
}

void u_lock(void* data)
{
    if ( data )
    {
        pthread_mutex_lock( (pthread_mutex_t*)data );
    }
}


void u_destroy_lock(void* data)
{
    if ( data )
    {
        pthread_mutex_destroy((pthread_mutex_t*)data);
    }
}

void u_unlock(void* data)
{
    if ( data )
    {	
        pthread_mutex_unlock((pthread_mutex_t*)data);
    }
}



