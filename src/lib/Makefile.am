AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""

wsmanincludedir = $(includedir)/openwsman
wsmaninclude_HEADERS = \
	wsman-types.h \
	wsman-names.h \
	wsman-faults.h \
	wsman-debug.h \
	wsman-errors.h \
	wsman-xml-api.h \
	wsman-client-api.h \
	wsman-client-transport.h \
	wsman-xml-serializer.h \
    wsman-apache.h


OWSMAN_INCLUDES = \
	wsman-soap.h \
	wsman-soap-envelope.h \
	wsman-xml.h \
	wsman-soap.h \
	wsman-soap-message.h \
	wsman-catalog.h \
	wsman-server.h \
	wsman-plugins.h \
	wsman-dispatcher.h \
	wsman-faults.h \
	wsman-xml.h \
	wsman-xml-binding.h \
	wsman-xml-serialize.h \
	wsman-client.h  \
	wsman-soap-message.h \
	$(wsmaninclude_HEADERS)

	
libwsman_catalog_la_SOURCES = 

libwsman_server_la_SOURCES = \
	wsman-server.c  \
	wsman-plugins.c \
    wsman-apache.c

libwsman_la_SOURCES = \
	wsman-xml-binding.c \
	wsman-xml.c \
	wsman-dispatcher.c \
	wsman-soap.c \
	wsman-faults.c \
	wsman-xml-serialize.c \
	wsman-soap-envelope.c \
	wsman-debug.c \
	wsman-soap-message.c \
	$(OWSMAN_INCLUDES)


libwsman_curl_client_transport_la_SOURCES = \
		wsman-client-transport.c \
		wsman-curl-client-transport.c


libwsman_client_la_SOURCES = \
	wsman-client.c 

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/src/libu \
	   $(XML_CFLAGS) 

LIBS = \
        $(XML_LIBS)  \
       $(top_builddir)/src/libu/libwsman_utils.la 



libwsman_curl_client_transport_la_LIBADD = $(CURL_LIBS)
libwsman_curl_client_transport_la_LDFLAGS = -version-info 1:0

libwsman_la_LIBADD = $(LIBS) -lpthread
libwsman_la_LDFLAGS= -version-info 1:0

libwsman_client_la_LIBADD = $(LIBS) libwsman_curl_client_transport.la
libwsman_client_la_LDFLAGS= -version-info 1:0

libwsman_server_la_LIBADD = -ldl
libwsman_server_la_LDFLAGS= -version-info 1:0



libwsman_catalog_la_LDFLAGS= -version-info 1:0

lib_LTLIBRARIES = \
		 libwsman.la \
		 libwsman_curl_client_transport.la \
		 libwsman_client.la \
		 libwsman_catalog.la \
		 libwsman_server.la

