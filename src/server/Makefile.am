AM_CFLAGS = @CFLAGS@  -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\"" -DPACKAGE_AUTH_DIR="\"${PACKAGE_AUTH_DIR}\""  -DPACKAGE_SUBSCRIPTION_DIR="\"${PACKAGE_SUBSCRIPTION_DIR}\""   

INCLUDES =  \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/src/server/shttpd \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" \
	   -DEMBEDDED  -DNO_CGI -DNO_SSI 


LIBS = \
       $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_server.la 

if ENABLE_EVENTING_SUPPORT
LIBS +=  $(top_builddir)/src/lib/libwsman_client.la
endif

if USE_OPENSSL
INCLUDES += $(OPENSSL_CFLAGS) 
LIBS += -lssl
endif



sbin_PROGRAMS = openwsmand

openwsmand_SOURCES = \
		shttpd/string.c \
		shttpd/shttpd.c \
		shttpd/auth.c \
		shttpd/md5.c \
		shttpd/adapter.c \
		shttpd/cgi.c \
		shttpd/mime_type.c shttpd/config.c shttpd/io_socket.c \
		shttpd/io_ssl.c shttpd/io_emb.c shttpd/compat_unix.c shttpd/io_dir.c shttpd/io_file.c \
		shttpd/shttpd_defs.h shttpd/llist.h shttpd/shttpd.h shttpd/std_includes.h shttpd/io.h shttpd/md5.h shttpd/ssl.h \
		shttpd/compat_unix.h shttpd/compat_win32.h shttpd/compat_rtems.h shttpd/adapter.h\
		wsmand-listener.h \
		wsmand-daemon.c \
		wsmand-daemon.h \
		wsmand-listener.c \
		gss.c \
		wsmand.c 

		

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(PACKAGE_SUBSCRIPTION_DIR)
