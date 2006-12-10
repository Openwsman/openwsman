AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""
SUBDIRS = . u test

libwsman_utils_la_SOURCES = buf.c env.c  \
		log.c memory.c misc.c str.c  uri.c  uuid.c \
		lock.c strings.c url2.c md5.c  list.c hash.c base64.c \
		strlib.c dictionary.c iniparser.c debug.c uerr.c \
		uoption.c gettimeofday.c syslog.c strsep.c strtok_r.c \
		pthreadx_win32.c os.c

INCLUDES = -I$(top_srcdir) 

LIBS = -lpthread

libwsman_utils_la_LIBADD = $(LIBS)

lib_LTLIBRARIES= \
		 libwsman_utils.la 


