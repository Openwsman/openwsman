wsmanincludedir = $(includedir)/openwsman
wsmaninclude_HEADERS = $(OWSMAN_INCLUDES)
OWSMAN_INCLUDES = \
	soap_api.h \
	ws_dispatcher.h \
	ws_errors.h \
	wsman-debug.h \
	wsman-faults.h \
	ws_utilities.h \
	ws_xml_api.h \
	xml_api_generic.h \
	xml_binding_libxml2.h \
	xml_serialization.h \
	xml_serializer.h \
	wsman-client.h  \
	wsman-md5-utils.h
		      

libwsman_la_SOURCES = \
	xml_binding_libxml2.c \
	xml_api_generic.c \
	xml_serialization.c \
	ws_dispatcher.c \
	ws_soap_api.c \
	ws_utilities.c \
	wsman-debug.c \
	wsman-faults.c \
	wsman-client.c \
	wsman-md5-utils.c \
	$(OWSMAN_INCLUDES)

INCLUDES = -I$(top_srcdir) \
	   $(XML_CFLAGS) \
	   $(GLIB_CFLAGS)

LIBS = \
	$(GLIB_LIBS)	\
        $(XML_LIBS)


libwsman_la_LIBADD =       \
	$(LIBS)
	-lpthread

libwsman_la_LDFLAGS= -version-info 1:0

lib_LTLIBRARIES= libwsman.la
