/* 
 * Copyright (c) 2005, 2006 by KoanLogic s.r.l. - All rights reserved.  
 */
#ifndef _LIBU_OS_H_
#define _LIBU_OS_H_


#include <u/pthreadx.h>
#include <u/syslog.h>
#include <u/gettimeofday.h>
#include <u/syslog.h>
#include <u/getpid.h>
#include <u/strsep.h>
#include <u/strtok_r.h>


#if defined WIN32 && ! defined __CYGWIN__
#define strcasecmp      stricmp
#define strncasecmp     strnicmp

#include <windows.h>
#include <wtypes.h>

const char * getpass (const char *);
#define dlclose(handle)         FreeLibrary(handle)
        
#define sleep(secs) Sleep( (secs) * 1000 )
#define snprintf _snprintf              /*!< The snprintf is called _snprintf() in Win32 */
#define popen _popen
#define getpid GetCurrentProcessId
#define pclose _pclose
#ifndef ssize_t
typedef int ssize_t;
#endif

#define bzero(p, l) memset(p, 0, l)

#endif

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
#  endif

#ifndef _PASSWORD_LEN
#define _PASSWORD_LEN	128
#endif

#ifndef HAVE_SSIZE_T
// typedef int ssize_t;
#endif

/* on VxWorks/DCC there's not extern declaration (even if the var is available
   into library files) */
extern char *optarg;
extern int optind;

#endif 
