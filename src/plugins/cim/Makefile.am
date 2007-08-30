plugindir = @PACKAGE_PLUGIN_DIR@

libwsman_cim_plugin_la_SOURCES = \
	sfcc-interface.c \
	sfcc-interface.h \
	cim_data.c \
	cim_data_stubs.c \
	cim_data.h

INCLUDES = -I$(top_srcdir)/include \
	   -I$(top_srcdir)/include/cim \
	   $(SFCC_INCLUDES) 

LIBS = \
    $(XML_LIBS) \
    $(top_builddir)/src/lib/libwsman.la

plugin_LTLIBRARIES= libwsman_cim_plugin.la

libwsman_cim_plugin_la_LIBADD =       \
        $(XML_LIBS)   \
       $(top_builddir)/src/lib/libwsman.la \
	-lpthread \
	-lcmpisfcc

libwsman_cim_plugin_la_LDFLAGS= -version-info 1:0

