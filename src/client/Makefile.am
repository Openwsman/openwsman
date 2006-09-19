
INCLUDES = \
	   $(XML_CFLAGS) \
	   $(SOUP_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu

LIBS = \
       $(XML_LIBS) \
       $(SOUP_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/libu/libwsman_utils.la


wsman_SOURCES = wsman.c \
		wsman-client-options.c \
		wsman-client-options.h \
		wsman.h
		
bin_PROGRAMS = \
	wsman
   


