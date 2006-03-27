
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


test_this_SOURCES = client.c \
		     test_this.c
		

test2_put_SOURCES = client.c \
		    test2_put.c

noinst_PROGRAMS = \
		  test_this \
		  test2_put
	
   


