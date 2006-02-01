plugindir = @PACKAGE_PLUGIN_DIR@

libIpmi_Sel_la_SOURCES = \
	IpmiSel_data.c \
	IpmiSel_data_stubs.c \
	IpmiSel_data.h

INCLUDES = -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/plugins/cim/lib \
	   -I/usr/local/include/CimClientLib \
	   $(GMODULE_CFLAGS)

LIBS = \
    $(XML_LIBS) \
    $(GMODULE_LIBS) \
    $(top_builddir)/src/plugins/cim/lib/libwsmanCimIfc.la

plugin_LTLIBRARIES= libIpmi_Sel.la

libIpmi_Sel_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread

libIpmi_Sel_la_LDFLAGS= -version-info 1:0

