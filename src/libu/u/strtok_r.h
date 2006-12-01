/* 
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.  
 */
#ifndef _LIBU_STRTOK_R_H_
#define _LIBU_STRTOK_R_H_
#include "libu_conf.h"

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

char *strtok_r(char *s, const char *delim, char **last);

#ifdef __cplusplus
}
#endif

#endif 

#endif
