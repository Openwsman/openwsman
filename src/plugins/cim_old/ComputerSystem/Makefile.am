plugindir = @PACKAGE_PLUGIN_DIR@

libComputerSystem_data_la_SOURCES = \
	ComputerSystem_data.c \
	ComputerSystem_data_stubs.c \
	ComputerSystem_data.h

INCLUDES = -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/plugins/cim/lib \
	   -I/usr/local/include/CimClientLib \
	   $(GMODULE_CFLAGS)

LIBS = \
    $(XML_LIBS) \
    $(GMODULE_LIBS) \
    $(top_builddir)/src/plugins/cim/lib/libwsmanCimIfc.la

plugin_LTLIBRARIES= libComputerSystem_data.la

libComputerSystem_data_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread

libComputerSystem_data_la_LDFLAGS= -version-info 1:0

