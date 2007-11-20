AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\"" -DPACKAGE_AUTH_DIR="\"${PACKAGE_AUTH_DIR}\""  -DPACKAGE_SUBSCRIPTION_DIR="\"${PACKAGE_SUBSCRIPTION_DIR}\""

INCLUDES =  \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\"

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
		 wsmand-listener.h \
		 wsmand-daemon.c \
		 wsmand-daemon.h \
                 wsmand-auth.c \
		 wsmand-auth.h \
		 wsmand.c 

openwsmand_SOURCES += \
                 shttpd.h \
                 shttpd.c \
		 wsmand-listener.c

		

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(PACKAGE_SUBSCRIPTION_DIR)
