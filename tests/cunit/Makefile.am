AM_CFLAGS = @CFLAGS@

INCLUDES = \
	   $(XML_CFLAGS) \
           $(LIBXSLT_CFLAGS) \
           -I/usr/local/include \
           -I$(top_srcdir)/src/plugins \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
           -I$(top_srcdir)/src/libu	

LIBS = \
       $(XML_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/libu/libwsman_utils.la \
       /usr/local/lib/libcunit.a

if USE_LIBCURL
LIBS += $(CURL_LIBS)
endif

if USE_LIBSOUP_CLIENT
LIBS += $(SOUP_LIBS) \
	    $(GOBJECT_LIBS) \
	    $(GTHREAD_LIBS)
endif

run_tests_SOURCES = run_tests.c \
              client_suite.c \
              test_enumeration.c

noinst_PROGRAMS = run_tests 

