AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""

INCLUDES = \
	   $(XML_CFLAGS) \
	   $(GMODULE_CFLAGS) \
	   $(SOUP_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib

LIBS = \
       $(XML_LIBS) \
       $(GMODULE_LIBS) \
       $(SOUP_LIBS) \
       $(top_builddir)/src/lib/libwsman.la


wsmand_SOURCES = \
		 wsmand-listener.h \
		 wsmand-plugins.h \
		 wsmand-listener.c \
		 wsmand-plugins.c \
		 wsmand-daemon.c \
		 wsmand-daemon.h \
		 wsmand.c 


sbin_PROGRAMS = \
	wsmand
   
