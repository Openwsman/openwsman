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
	
ser1_tests_LDADD = \
       $(OPENWSMAN_LIBS) -lcunit


ser1_tests_SOURCES = ser1.c

noinst_PROGRAMS = ser1_tests 
