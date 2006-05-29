cimifcincludedir = $(includedir)/openwsman/cim
INCLUDES = -I$(top_srcdir) \
	   $(XML_CFLAGS) \
	   $(GLIB_CFLAGS) \
	   $(GMODULE_CFLAGS) \
	   -I$(top_srcdir)/src/lib

cimifc_INCLUDES  = sfcc-interface_utils.h sfcc-interface.h

libwsmanCimIfc_la_SOURCES = \
	sfcc-interface.c \
	sfcc-interface_utils.c \
	sfcc-interface_utils.h \
	sfcc-interface.h

LIBS = \
	$(GLIB_LIBS)	\
        $(XML_LIBS) \
	$(GMODULE_LIBS) \
       $(top_builddir)/src/lib/libwsman.la


libwsmanCimIfc_la_LIBADD =       \
	$(GLIB_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
	-lcmpisfcc

libwsmanCimIfc_la_LDFLAGS= -version-info 1:0

lib_LTLIBRARIES=  libwsmanCimIfc.la
