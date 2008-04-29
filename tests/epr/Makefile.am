
INCLUDES = \
	   $(XML_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/include

LIBS = \
       $(XML_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la\
	-lcunit


test_epr_SOURCES = test_epr.c
test_WsmanEPR_SOURCES = test_WsmanEPR.cpp

noinst_PROGRAMS = \
		  test_epr\
		  test_WsmanEPR	
	
   

