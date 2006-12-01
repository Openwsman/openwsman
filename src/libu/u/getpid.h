/* 
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.  
 */
#ifndef _LIBU_GETPID_H_
#define _LIBU_GETPID_H_
#include "libu_conf.h"

#ifdef  WIN32
    #include <windows.h>
    typedef DWORD pid_t;
#endif


#endif
