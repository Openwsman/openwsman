
libwsman_this_la_SOURCES = \
	wsman_this.c \
	wsman_this_stubs.c \
	wsman_this.h

INCLUDES = -I$(top_srcdir)/src/lib

LIBS = \
    $(XML_LIBS)

lib_LTLIBRARIES= libwsman_this.la

libwsman_this_la_LIBADD =       \
        $(XML_LIBS)   \
	-lpthread

libwsman_this_la_LDFLAGS= -version-info 1:0

