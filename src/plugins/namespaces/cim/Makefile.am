plugindir = @PACKAGE_PLUGIN_DIR@

libcim_namespace_la_SOURCES = \
	cim_namespace_data.c \
	cim_namespace_data_stubs.c \
	cim_namespace_data.h

INCLUDES = -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/plugins/cim/lib \
	   -I/usr/local/include/CimClientLib \
	   $(GMODULE_CFLAGS)

LIBS = \
    $(XML_LIBS) \
    $(GMODULE_LIBS) \
    $(top_builddir)/src/plugins/cim/lib/libwsmanCimIfc.la

plugin_LTLIBRARIES= libcim_namespace.la

libcim_namespace_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread

libcim_namespace_la_LDFLAGS= -version-info 1:0

