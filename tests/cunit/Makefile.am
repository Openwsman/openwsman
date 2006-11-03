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

LIBS += $(CURL_LIBS)

run_tests_SOURCES = run_tests.c \
		client_suite.h \
              client_suite.c \
              enumeration.h \
              enumeration.c

noinst_PROGRAMS = run_tests 

