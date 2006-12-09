SUBDIRS = . u test

libwsman_utils_la_SOURCES = buf.c env.c  \
		log.c memory.c misc.c str.c  uri.c   \
		lock.c strings.c url2.c md5.c  list.c hash.c base64.c \
		strlib.c dictionary.c iniparser.c debug.c uerr.c \
		uoption.c gettimeofday.c syslog.c strsep.c strtok_r.c \
		pthreadx_win32.c os.c  uuid_clear.c \
		uuid.c \
		uuid_compare.c \
		uuid_copy.c \
		uuid_isnull.c \
		uuid_pack.c \
		uuid_parse.c \
		uuid_time.c \
		uuid_unpack.c \
		uuid_unparse.c \
		gen_uuid.c \
		gen_uuid_nt.c

INCLUDES = -I$(top_srcdir) 

LIBS = -lpthread

libwsman_utils_la_LIBADD = $(LIBS)

lib_LTLIBRARIES= \
		 libwsman_utils.la 


