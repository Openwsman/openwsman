/*
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.
 */
#ifndef _LIBU_OS_H_
#define _LIBU_OS_H_


#include <u/pthreadx.h>
#include <u/syslog.h>
#include <u/gettimeofday.h>


#if defined (__FreeBSD__)  || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)
struct __timezone {
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

#endif

#if defined WIN32 && ! defined __CYGWIN__
#define strcasecmp      stricmp
#define strncasecmp     strnicmp

#include <windows.h>
#include <wtypes.h>

typedef DWORD pid_t;

#ifdef __cplusplus
extern "C" {
#endif

pid_t getpid(void);

#ifdef __cplusplus
}
#endif

#define dlclose(handle)         FreeLibrary(handle)
#define strtoull(nptr, endptr, base) _strtoul_l(nptr, endptr, base, NULL)
#define strtoll(nptr, endptr, base) _strtol_l(nptr, endptr, base, NULL)
#define sleep(secs) Sleep( (secs) * 1000 )
#define snprintf _snprintf              /*!< The snprintf is called _snprintf() in Win32 */
#define popen _popen
#define getpid GetCurrentProcessId
#define pclose _pclose
#ifndef ssize_t
typedef int ssize_t;
#endif

#define bzero(p, l) memset(p, 0, l)

#endif // WIN32



/* Define VA_COPY() to do the right thing for copying va_list variables. */
#ifdef WIN32
#  if defined (__GNUC__) && defined (__PPC__) && (defined (_CALL_SYSV) || defined (_WIN32))
#    define VA_COPY(ap1, ap2) (*(ap1) = *(ap2))
#  elif defined (VA_COPY_AS_ARRAY)
#    define VA_COPY(ap1, ap2) i_memmove ((ap1), (ap2), sizeof (va_list))
#  else /* va_list is a pointer */
#    define VA_COPY(ap1, ap2) ((ap1) = (ap2))
#  endif /* va_list is a pointer */
#else
# define VA_COPY va_copy
#endif

#ifdef __GNUC__
#define __INLINE__ __inline__
#elif _WIN32
#define __INLINE__ __inline
#elif __SUNPRO_C || __SUNPRO_CC
#define __INLINE__ inline
#  endif

#ifndef _PASSWORD_LEN
#define _PASSWORD_LEN	128
#endif


#if defined(__vxworks) || defined(__VXWORKS__)
/* on VxWorks/DCC there's not extern declaration (even if the var is available
   into library files) */
extern char *optarg;
extern int optind;
#endif




#ifdef WIN32

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_STRSEP
char * strsep(char **, const char *);
#endif



char *strtok_r(char *s, const char *delim, char **last);

const char * getpass (const char *);

#ifdef __cplusplus
}
#endif

#endif




#endif
