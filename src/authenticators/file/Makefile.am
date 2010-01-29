authdir = @PACKAGE_AUTH_DIR@

LIBS = $(top_builddir)/src/lib/libwsman.la

libwsman_file_auth_la_SOURCES = \
	file_auth.c

INCLUDES = -I$(top_srcdir)/include

libwsman_file_auth_la_LIBADD = $(CRYPT_LIBS)


auth_LTLIBRARIES= libwsman_file_auth.la

libwsman_file_auth_la_LDFLAGS = -version-info 1:0
