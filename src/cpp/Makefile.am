
wsmanincludedir = $(includedir)/openwsman/cpp

INCLUDES =  \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\"

LIBS = \
    $(XML_LIBS)  \
    $(top_builddir)/src/libu/libwsman_utils.la \
    $(top_builddir)/src/lib/libwsman.la \
    $(top_builddir)/src/lib/libwsman_client.la 

wsmaninclude_HEADERS = \
		       WsmanClient.h  \
		       Exception.h

lib_LTLIBRARIES=libwsman_clientpp.la

libwsman_clientpp_la_SOURCES = \
	WsmanClient.cpp  \
	WsmanClient.h \
	Exception.cpp \
	Exception.h


libwsman_clientpp_la_CPPFLAGS = 




