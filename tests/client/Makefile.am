
INCLUDES = \
	   $(XML_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
		-I$(top_srcdir)/src/libu	

LIBS = \
       $(XML_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/libu/libwsman_utils.la

if USE_LIBCURL
LIBS += $(CURL_LIBS)
endif

if USE_LIBSOUP_CLIENT
LIBS += $(SOUP_LIBS) \
	    $(GOBJECT_LIBS) \
	    $(GTHREAD_LIBS)
endif

test_transfer_get_SOURCES = test_transfer_get.c
test_enumeration_SOURCES = test_enumeration.c
test_pull_SOURCES = test_pull.c

noinst_PROGRAMS = \
		  test_transfer_get \
		  test_enumeration \
		  test_pull
	
   


