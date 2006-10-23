AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""

wsmanincludedir = $(includedir)/openwsman
wsmaninclude_HEADERS = $(OWSMAN_INCLUDES)
OWSMAN_INCLUDES = \
	wsman-soap.h \
	wsman-soap-envelope.h \
	wsman-soap-message.h \
	wsman-catalog.h \
	wsman-server.h \
	wsman-plugins.h \
	wsman-dispatcher.h \
	wsman-errors.h \
	wsman-faults.h \
	wsman-xml-api.h \
	wsman-xml.h \
	wsman-xml-binding.h \
	wsman-xml-serialize.h \
	wsman-xml-serializer.h \
	wsman-client.h  \
	wsman-soap-message.h \
	wsman-client-transport.h

	
libwsman_catalog_la_SOURCES = 

libwsman_server_la_SOURCES = \
	wsman-server.c  \
	wsman-plugins.c 

libwsman_la_SOURCES = \
	wsman-xml-binding.c \
	wsman-xml.c \
	wsman-dispatcher.c \
	wsman-soap.c \
	wsman-faults.c \
	wsman-xml-serialize.c \
	wsman-soap-envelope.c \
	wsman-soap-message.c
	$(OWSMAN_INCLUDES)


if USE_LIBCURL
libwsman_curl_client_transport_la_SOURCES = \
		wsman-client-transport.c \
		wsman-curl-client-transport.c
libwsman_curl_client_transport_la_LIBADD = $(CURL_LIBS)
libwsman_curl_client_transport_la_LDFLAGS= -version-info 1:0
endif

if USE_LIBSOUP_CLIENT
libwsman_soup_client_transport_la_SOURCES = \
		wsman-client-transport.c \
		wsman-soup-client-transport.c
libwsman_soup_client_transport_la_LIBADD = $(SOUP_LIBS) \
       $(GOBJECT_LIBS) \
       $(GTHREAD_LIBS)
libwsman_soup_client_transport_la_LDFLAGS= -version-info 1:0
endif

libwsman_client_la_SOURCES = \
	wsman-client.c 

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src/libu \
	   $(XML_CFLAGS) 

LIBS = \
        $(XML_LIBS)  \
       $(top_builddir)/src/libu/libwsman_utils.la 




libwsman_la_LIBADD = $(LIBS) \
	-lpthread

libwsman_client_la_LIBADD = $(LIBS)
libwsman_server_la_LIBADD = -ldl

libwsman_la_LDFLAGS= -version-info 1:0
libwsman_server_la_LDFLAGS= -version-info 1:0
libwsman_catalog_la_LDFLAGS= -version-info 1:0

lib_LTLIBRARIES = \
		 libwsman.la 


if USE_LIBCURL
lib_LTLIBRARIES += libwsman_curl_client_transport.la 
libwsman_client_la_LIBADD += libwsman_curl_client_transport.la

endif

if USE_LIBSOUP_CLIENT
lib_LTLIBRARIES += libwsman_soup_client_transport.la 
INCLUDES += $(SOUP_CFLAGS) \
	    $(GOBJECT_CFLAGS) \
	    $(GTHREAD_CFLAGS)

libwsman_client_la_LIBADD +=  libwsman_soup_client_transport.la
endif
lib_LTLIBRARIES += \
		 libwsman_client.la \
		 libwsman_catalog.la \
		 libwsman_server.la

libwsman_client_la_LDFLAGS= -version-info 1:0
