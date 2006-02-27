plugindir = @PACKAGE_PLUGIN_DIR@

libIpmi_LogRecord_la_SOURCES = \
	IpmiLogRecord_data.c \
	IpmiLogRecord_data_stubs.c \
	IpmiLogRecord_data.h

INCLUDES = -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/plugins/cim/lib \
	   -I/usr/local/include/CimClientLib \
	   $(GMODULE_CFLAGS)

LIBS = \
    $(XML_LIBS) \
    $(GMODULE_LIBS) \
    $(top_builddir)/src/plugins/cim/lib/libwsmanCimIfc.la

plugin_LTLIBRARIES= libIpmi_LogRecord.la

libIpmi_LogRecord_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread

libIpmi_LogRecord_la_LDFLAGS= -version-info 1:0

