
INCLUDES = \
	   $(XML_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/include

LIBS = \
       $(XML_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la\
       -lcunit

test_filter_SOURCES = test_filter.c
test_WsmanFilter_SOURCES = test_WsmanFilter.cpp

noinst_PROGRAMS = \
		  test_filter\
		  test_WsmanFilter
	
   


