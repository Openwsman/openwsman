plugindir = @PACKAGE_PLUGIN_DIR@

libIpmiSensor_data_la_SOURCES = \
	IpmiSensor_data.c \
	IpmiSensor_data_stubs.c \
	IpmiSensor_data.h

INCLUDES = -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/plugins/cim/lib \
	   -I/usr/local/include/CimClientLib \
	   $(GMODULE_CFLAGS)

LIBS = \
    $(XML_LIBS) \
    $(GMODULE_LIBS) \
    $(top_builddir)/src/plugins/cim/lib/libwsmanCimIfc.la

plugin_LTLIBRARIES= libIpmiSensor_data.la

libIpmiSensor_data_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread
libIpmiSensor_data_la_LDFLAGS= -version-info 1:0

