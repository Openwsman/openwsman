
wsmanincludedir = $(includedir)/openwsman/cpp

INCLUDES =  \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	    $(OPENSSL_CFLAGS)

LIBS = \
    $(XML_LIBS)  \
    $(top_builddir)/src/lib/libwsman.la \
    $(top_builddir)/src/lib/libwsman_client.la 

wsmaninclude_HEADERS = \
		       OpenWsmanClient.h  \
		       Exception.h \
			WsmanClient.h \
			WsmanEPR.h \
			WsmanFilter.h

lib_LTLIBRARIES=libwsman_clientpp.la

libwsman_clientpp_la_SOURCES = \
	OpenWsmanClient.cpp  \
	OpenWsmanClient.h \
	WsmanEPR.h \
	WsmanFilter.h \
	Exception.h \
	WsmanClient.h


libwsman_clientpp_la_CPPFLAGS = 




