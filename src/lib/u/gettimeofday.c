
#ifdef HAVE_CONFIG_H
#include <wsman_config.h>
#endif


#ifndef HAVE_GETTIMEOFDAY
#include <u/carpal.h>
#include <u/gettimeofday.h>

#ifdef WIN32
#include <time.h>
#include <sys/timeb.h>
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    struct _timeb tb;

    dbg_return_if(tv == NULL, -1);

    /* get current time */
    _ftime(&tb);

    /* set the timeval struct */
    tv->tv_sec = (long)tb.time;
    tv->tv_usec = 1000 * tb.millitm;

    if(tz == NULL)
        return 0;

    /* set the tiemzone struct */
    tz->tz_minuteswest = tb.timezone;
    tz->tz_dsttime = tb.dstflag;

    return 0;
}
#else
#warning missing gettimeofday,tv.tv_usec will be always set to zero
int gettimeofday(struct timeval *tv, struct timezone *tzp)
{
	if(tzp)
		tzp->tz_minuteswest = tzp->tz_dsttime = 0;

	tv->tv_sec = time(0);
	tv->tv_usec = 0;

	return 0;
}
#endif

#else
#include <sys/time.h>
/* int gettimeofday(struct timeval *tp, struct timezone *tzp); */
#endif
