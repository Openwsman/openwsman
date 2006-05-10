
cimifcincludedir = $(includedir)/openwsman/cim
cimifc_INCLUDES  = interface_utils.h sfcc-interface.h

libwsmanCimIfc_la_SOURCES = \
	sfcc-interface.c \
	interface_utils.c \
	interface_utils.h \
	sfcc-interface.h

INCLUDES = -I$(top_srcdir) \
	   $(GLIB_CFLAGS) \
	   -I$(top_srcdir)/src/lib


LIBS = \
	$(GLIB_LIBS)

lib_LTLIBRARIES= libwsmanCimIfc.la

libwsmanCimIfc_la_LIBADD =       \
	$(GLIB_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
	-lcmpisfcc

libwsmanCimIfc_la_LDFLAGS= -version-info 1:0
