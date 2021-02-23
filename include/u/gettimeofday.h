/*
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.
 */
#ifndef _LIBU_GETTIMEOFDAY_H_
#define _LIBU_GETTIMEOFDAY_H_

#include <time.h>
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif

#if defined (WIN32) && ! defined (__MINGW32__)

#ifdef __cplusplus
extern "C" {
#endif

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

int gettimeofday(struct timeval *tp, struct timezone *tzp);

#ifdef __cplusplus
}
#endif

#endif /* HAVE_GETTIMEOFDAY */

#endif
