
plugindir = @PACKAGE_PLUGIN_DIR@

libwsman_catalog_la_SOURCES = \
	wsman_catalog.c \
	wsman_catalog_stubs.c \
	wsman_catalog.h

INCLUDES = -I$(top_srcdir)/src/lib \
	   $(GMODULE_CFLAGS)

LIBS = \
    $(XML_LIBS) \
    $(GMODULE_LIBS)

plugin_LTLIBRARIES= libwsman_catalog.la

libwsman_catalog_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread

libwsman_catalog_la_LDFLAGS= -version-info 1:0

