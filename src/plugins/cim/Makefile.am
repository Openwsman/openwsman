plugindir = @PACKAGE_PLUGIN_DIR@

libwsman_cim_plugin_la_SOURCES = \
	cim_data.c \
	cim_data_stubs.c \
	cim_data.h

INCLUDES = -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu \
	   -I$(top_srcdir)/src/libcim/sfcc \
	   -I$(top_srcdir)/src/libcim/include \
	   $(SFCC_INCLUDES) 

LIBS = \
    $(XML_LIBS) \
    $(GMODULE_LIBS) \
    $(top_builddir)/src/libcim/sfcc/libwsmanCimIfc.la

plugin_LTLIBRARIES= libwsman_cim_plugin.la

libwsman_cim_plugin_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread

libwsman_cim_plugin_la_LDFLAGS= -version-info 1:0

