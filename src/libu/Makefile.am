AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""
SUBDIRS = . u test

wsmanuincludedir = $(includedir)/openwsman/u
wsmanuinclude_HEADERS = libu_conf.h

libwsman_utils_la_SOURCES = buf.c config.c env.c fnmatch.c \
		log.c memory.c misc.c str.c  unlink.c uri.c  uuid.c \
		lock.c strings.c url2.c md5.c  list.c hash.c base64.c \
		strlib.c dictionary.c iniparser.c debug.c uerr.c ulist.c \
		uoption.c

INCLUDES = -I$(top_srcdir) 

LIBS = -lpthread

libwsman_utils_la_LIBADD = $(LIBS)

lib_LTLIBRARIES= \
		 libwsman_utils.la 


