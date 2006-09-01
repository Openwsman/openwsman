AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""

wsmanincludedir = $(includedir)/openwsman
wsmaninclude_HEADERS = $(OWSMAN_INCLUDES)
OWSMAN_INCLUDES = \
	wsman-soap.h \
	wsman-dispatcher.h \
	wsman-errors.h \
	wsman-debug.h \
	wsman-faults.h \
	wsman-util.h \
	wsman-xml-api.h \
	wsman-xml.h \
	wsman-xml-binding.h \
	wsman-xml-serialize.h \
	wsman-xml-serializer.h \
	wsman-client.h  \
	wsman-soap-message.h \
	wsman-md5-utils.h

libwsman_utils_la_SOURCES = \
	wsman-md5-utils.c \
	wsman-util.c \
	wsman-debug.c
		      
libwsman_client_la_SOURCES = \
	wsman-client.c 
	
libwsman_catalog_la_SOURCES = \
	wsman-catalog.c 

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
	   $(XML_CFLAGS) \
	   $(GLIB_CFLAGS)

LIBS = \
	$(GLIB_LIBS)	\
        $(XML_LIBS)


libwsman_la_LIBADD = $(LIBS) \
	-lpthread

libwsman_client_la_LIBADD = $(LIBS)
libwsman_utils_la_LIBADD = $(LIBS)

libwsman_la_LDFLAGS= -version-info 1:0
libwsman_client_la_LDFLAGS= -version-info 1:0
libwsman_utils_la_LDFLAGS= -version-info 1:0
libwsman_server_la_LDFLAGS= -version-info 1:0
libwsman_catalog_la_LDFLAGS= -version-info 1:0

lib_LTLIBRARIES= \
		 libwsman.la \
		 libwsman_client.la \
		 libwsman_utils.la \
		 libwsman_catalog.la \
		 libwsman_server.la
