AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""
SUBDIRS = . u test

libwsman_utils_la_SOURCES = buf.c  \
		log.c memory.c misc.c  uri.c  uuid.c \
		lock.c strings.c  md5.c  list.c hash.c base64.c \
		iniparser.c debug.c uerr.c \
		uoption.c gettimeofday.c syslog.c  \
		pthreadx_win32.c os.c

INCLUDES = -I$(top_srcdir) 

LIBS = -lpthread

libwsman_utils_la_LIBADD = $(LIBS)

lib_LTLIBRARIES= \
		 libwsman_utils.la 


