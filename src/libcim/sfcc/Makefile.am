cimifcincludedir = $(includedir)/openwsman/cim
INCLUDES = -I$(top_srcdir) \
	   $(XML_CFLAGS) \
	   -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu

cimifc_INCLUDES  = sfcc-interface_utils.h sfcc-interface.h

libwsmanCimIfc_la_SOURCES = \
	sfcc-interface.c \
	sfcc-interface.h

LIBS = \
        $(XML_LIBS) \
       $(top_builddir)/src/lib/libwsman.la


libwsmanCimIfc_la_LIBADD =       \
       $(top_builddir)/src/lib/libwsman.la \
	-lcmpisfcc -lcimcClient  -lsfcUtil

libwsmanCimIfc_la_LDFLAGS= -version-info 1:0

lib_LTLIBRARIES=  libwsmanCimIfc.la
