
INCLUDES = \
	   $(XML_CFLAGS) \
	   $(LIBXSLT_CFLAGS) \
	   $(SOUP_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib

LIBS = \
       $(XML_LIBS) \
       $(LIBXSLT_LIBS) \
       $(SOUP_LIBS) \
       $(top_builddir)/src/lib/libwsman.la


wsman_SOURCES = wsman.c \
		parse_uri.c \
		transfer.c \
		enumeration.c \
		utils.c \
		transport.c \
		catch.c \
		parse_uri.h \
		wsman.h
		


bin_PROGRAMS = \
	wsman
   


