
#if USE_PAM
authdir = @PACKAGE_AUTH_DIR@

libwsman_pam_auth_la_SOURCES = \
	pam_auth.c

INCLUDES = -I$(top_srcdir)/include

libwsman_pam_auth_la_LIBADD = -lpam
auth_LTLIBRARIES= libwsman_pam_auth.la

libwsman_pam_auth_la_LDFLAGS = -version-info 1:0

#endif
