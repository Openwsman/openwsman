
INCLUDES = \
	   $(XML_CFLAGS) \
	   $(LIBXSLT_CFLAGS) \
	   $(SOUP_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib

LIBS = \
       $(XML_LIBS) \
       $(SOUP_LIBS) \
       $(top_builddir)/src/lib/libwsman.la


wsman_this_SOURCES = client.c \
		     wsman_this.c
		


noinst_PROGRAMS = \
		  wsman_this
	
   


