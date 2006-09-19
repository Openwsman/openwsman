/* 
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.  
 */
#ifndef _LIBU_GETPID_H_
#define _LIBU_GETPID_H_
#include "libu_conf.h"

#ifdef HAVE_GETPID
#include <sys/types.h>
#include <unistd.h>
#else /* !HAVE_GETPID */

#ifdef _WIN32
    #include <windows.h>
    typedef DWORD pid_t;
#else /* !_WIN32 */
    typedef unsigned int pid_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

pid_t getpid(void);

#ifdef __cplusplus
}
#endif

#endif

#endif
