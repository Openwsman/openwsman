authdir = @PACKAGE_AUTH_DIR@


libwsman_file_auth_la_SOURCES = \
	file_auth.c

INCLUDES = -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu

#if LIBCRYPT
libwsman_file_auth_la_LIBADD = -lcrypt
#endif


auth_LTLIBRARIES= libwsman_file_auth.la

libwsman_file_auth_la_LDFLAGS = -version-info 1:0
