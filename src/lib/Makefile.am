AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""
SUBDIRS = . 

if DISABLE_SERVER
else
libwsman_server_la_SOURCES = \
	wsman-server.c  \
	wsman-plugins.c \
    	wsman-server-api.c
endif


UTIL_SOURCES = u/buf.c  \
		u/log.c u/memory.c u/misc.c  u/uri.c  u/uuid.c \
		u/lock.c u/md5.c u/strings.c u/list.c u/hash.c u/base64.c \
		u/iniparser.c u/debug.c u/uerr.c \
		u/uoption.c u/gettimeofday.c u/syslog.c  \
		u/pthreadx_win32.c u/os.c

libwsman_la_SOURCES = \
	$(UTIL_SOURCES) \
	wsman-libxml2-binding.c \
	wsman-xml.c \
	wsman-epr.c \
	wsman-filter.c \
	wsman-dispatcher.c \
	wsman-soap.c \
	wsman-faults.c \
	wsman-xml-serialize.c \
	wsman-soap-envelope.c \
	wsman-debug.c \
	wsman-soap-message.c 

if ENABLE_EVENTING_SUPPORT
libwsman_la_SOURCES +=  \
	wsman-subscription-repository.c \
	wsman-event-pool.c \
	wsman-cimindication-processor.c
endif

libwsman_curl_client_transport_la_SOURCES = \
	wsman-client-transport.c \
	wsman-curl-client-transport.c


libwsman_client_la_SOURCES = \
	wsman-client.c 

INCLUDES = -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   $(XML_CFLAGS) 

LIBS = \
        $(XML_LIBS) 



libwsman_curl_client_transport_la_LIBADD = $(CURL_LIBS)
libwsman_curl_client_transport_la_LDFLAGS = -version-info 1:0

libwsman_client_la_LIBADD = $(LIBS) libwsman_curl_client_transport.la
libwsman_client_la_LDFLAGS= -version-info 1:0

libwsman_la_LIBADD = $(LIBS) -lpthread 

if ENABLE_EVENTING_SUPPORT
libwsman_la_LIBADD += $(libwsman_client_la_LIBADD) libwsman_client.la
endif

libwsman_la_LDFLAGS= -version-info 1:0


if DISABLE_SERVER
else
libwsman_server_la_LIBADD = 
libwsman_server_la_LDFLAGS= -version-info 1:0
endif


lib_LTLIBRARIES = \
		 libwsman.la \
		 libwsman_curl_client_transport.la \
		 libwsman_client.la 
		 
if DISABLE_SERVER
else
lib_LTLIBRARIES += libwsman_server.la
endif

EXTRA_DIST = \
	wsman-win-client-transport.c
