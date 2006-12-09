AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""  -DPACKAGE_AUTH_DIR="\"${PACKAGE_AUTH_DIR}\""

INCLUDES =  \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\"

LIBS = \
       $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/libu/libwsman_utils.la \
       $(top_builddir)/src/lib/libwsman_server.la


if USE_OPENSSL
INCLUDES += $(OPENSSL_CFLAGS)
LIBS += -lssl
endif



sbin_PROGRAMS = wsmand

wsmand_SOURCES = \
		 wsmand-listener.h \
		 wsmand-daemon.c \
		 wsmand-daemon.h \
                 wsmand-auth.c \
		 wsmand-auth.h \
		 wsmand.c 

wsmand_SOURCES += \
                 shttpd.h \
                 shttpd.c \
		 wsmand-shttpd-listener.c

		

