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
	wsman-soap-message.h 

libwsman_client_la_SOURCES = \
	wsman-client.c 
	
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
libwsman_client_la_LDFLAGS= -version-info 1:0
libwsman_server_la_LDFLAGS= -version-info 1:0
libwsman_catalog_la_LDFLAGS= -version-info 1:0

lib_LTLIBRARIES= \
		 libwsman.la \
		 libwsman_client.la \
		 libwsman_catalog.la \
		 libwsman_server.la
