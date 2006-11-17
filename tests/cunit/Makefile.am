AM_CFLAGS = @CFLAGS@


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


LIBS += $(CURL_LIBS) $(CUNIT_LIBS)

	
wsman_cunit_tests_LDADD = \
       $(OPENWSMAN_LIBS) $(CUNIT_LIBS) -lcurl


wsman_cunit_tests_SOURCES = run_tests.c \
		    common.c \
		    common.h \
		    identify.c \
		    enumeration.c \
		    transfer_get.c

noinst_PROGRAMS = wsman_cunit_tests

