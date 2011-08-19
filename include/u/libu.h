/*
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.
 */

#ifndef _U_LIBU_H_
#define _U_LIBU_H_


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <u/carpal.h>
#include <u/log.h>
#include <u/logprv.h>
#include <u/memory.h>
#include <u/misc.h>
#include <u/buf.h>
#include <u/os.h>


#include <u/debug.h>
#include <u/debug_internal.h>

#include <u/hash.h>
#include <u/uri.h>
#include <u/uuid.h>
#include <u/lock.h>
#include <u/strings.h>
#include <u/uri.h>
#include <u/md5.h>
#include <u/list.h>
#include <u/base64.h>
#include <u/iniparser.h>
#include <u/uerr.h>
#include <u/uoption.h>

#ifdef WIN32
#define strdup _strdup
#define stricmp _stricmp
#define strnicmp _strnicmp
#define fileno _fileno
#define cputs _cputs
#endif
#ifndef TRUE
#define TRUE    1
#endif  /* TRUE */

#ifndef FALSE
#define FALSE    0
#endif  /* FALSE */

#endif /* !_U_LIBU_H_ */
