
INCLUDES = \
	   $(XML_CFLAGS) \
	   $(LIBXSLT_CFLAGS) \
	   $(SOUP_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu

LIBS = \
       $(XML_LIBS) \
       $(SOUP_LIBS) \
       $(top_builddir)/src/lib/libwsman.la


xml1_SOURCES = xml1.c 
xml2_SOURCES = xml2.c 

noinst_PROGRAMS = \
		  xml1  \
		  xml2
	
   


