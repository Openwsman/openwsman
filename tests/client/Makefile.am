
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


LIBS += $(CURL_LIBS)

test_references_SOURCES = test_references.c
test_transfer_get_SOURCES = test_transfer_get.c
test_transfer_put_SOURCES = test_transfer_put.c
test_invoke_SOURCES = test_invoke.c
test_identify_SOURCES = test_identify.c
test_enumeration_SOURCES = test_enumeration.c
test_pull_SOURCES = test_pull.c

noinst_PROGRAMS = \
		  test_references \
		  test_transfer_get \
		  test_transfer_put \
		  test_identify \
		  test_invoke \
		  test_enumeration \
		  test_pull
	
   


