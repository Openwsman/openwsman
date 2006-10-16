authdir = @PACKAGE_AUTH_DIR@

libwsman_pam_auth_la_SOURCES = \
	pam_auth.c

INCLUDES = -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu


auth_LTLIBRARIES= libwsman_pam_auth.la

libwsman_pam_auth_la_LDFLAGS = -version-info 1:0