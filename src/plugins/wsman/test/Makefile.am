
plugindir = @PACKAGE_PLUGIN_DIR@

libwsman_test_la_SOURCES = \
	wsman_test.c \
	wsman_test_stubs.c \
	wsman_test.h

INCLUDES = -I$(top_srcdir)/src/lib \
	   $(GMODULE_CFLAGS)

LIBS = \
    $(XML_LIBS) \
    $(GMODULE_LIBS)

plugin_LTLIBRARIES= libwsman_test.la

libwsman_test_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread

libwsman_test_la_LDFLAGS= -version-info 1:0

