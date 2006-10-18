AM_CFLAGS = @CFLAGS@

INCLUDES = \
	   $(XML_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu
       
if USE_LIBSOUP_CLIENT
INCLUDES += $(SOUP_CFLAGS) \
	    $(GOBJECT_CFLAGS) \
	    $(GTHREAD_CFLAGS)
endif
if USE_LIBCURL
INCLUDES += $(CURL_FLARS)
endif


LIBS = \
       $(XML_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/libu/libwsman_utils.la
if USE_LIBSOUP_CLIENT
LIBS += $(SOUP_LIBS) \
       $(GOBJECT_LIBS) \
       $(GTHREAD_LIBS)
    
endif
if USE_LIBCURL
LIBS += $(CURL_LIBS)
endif


wsman_SOURCES = wsman.c \
		wsman-client-options.c \
		wsman-client-options.h \
		wsman.h
		
bin_PROGRAMS = \
	wsman
   


