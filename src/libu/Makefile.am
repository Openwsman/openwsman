AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""
SUBDIRS = . u

wsmanuincludedir = $(includedir)/openwsman/u
wsmanuinclude_HEADERS = libu_conf.h

libwsman_utils_la_SOURCES = buf.c config.c daemon.c env.c fnmatch.c getpid.c gettimeofday.c log.c memory.c misc.c net.c str.c strsep.c strtok_r.c syslog.c timegm.c unlink.c uri.c  uuid.c locking.c strings.c url2.c md5.c  list.c hash.c base64.c strlib.c dictionary.c iniparser.c debug.c

INCLUDES = -I$(top_srcdir) 

LIBS = -lpthread

libwsman_utils_la_LIBADD = $(LIBS)

lib_LTLIBRARIES= \
		 libwsman_utils.la 


