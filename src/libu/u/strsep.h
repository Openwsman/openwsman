/* 
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.  
 */
#ifndef _LIBU_STRSEP_H_
#define _LIBU_STRSEP_H_
#include "libu_conf.h"

#ifdef _WIN32
#ifdef HAVE_STRSEP

#ifdef __cplusplus
extern "C" {
#endif

char * strsep(char **, const char *);

#ifdef __cplusplus
}
#endif

#endif
#endif

#endif
