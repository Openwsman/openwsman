AM_CFLAGS = 
INCLUDES = \
	   $(XML_CFLAGS) \
	   -I$(top_srcdir) \
		-I$(top_srcdir)/include

LIBS = \
       $(XML_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la 

serialize_SOURCES = serialize.c
	
noinst_PROGRAMS = \
	serialize


