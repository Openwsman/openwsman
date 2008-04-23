
INCLUDES = \
	   $(XML_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/include

LIBS = \
       $(XML_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la


LIBS += $(CURL_LIBS)

test_references_SOURCES = test_references.c
test_transfer_create_SOURCES = test_transfer_create.c
test_transfer_get_SOURCES = test_transfer_get.c
test_transfer_put_SOURCES = test_transfer_put.c
test_invoke_SOURCES = test_invoke.c
test_identify_SOURCES = test_identify.c
test_enumeration_SOURCES = test_enumeration.c
test_pull_SOURCES = test_pull.c
test_subscribe_SOURCES = test_subscribe.c
test_unsubscribe_SOURCES = test_unsubscribe.c
test_renew_SOURCES = test_renew.c
test_associators_SOURCES = test_associators.c
test_selectorfilter_SOURCES = test_selectorfilter.c

noinst_PROGRAMS = \
		  test_references \
		  test_transfer_create \
		  test_transfer_get \
		  test_transfer_put \
		  test_identify \
		  test_invoke \
		  test_enumeration \
		  test_associators \
		  test_selectorfilter \
		  test_pull \
		  test_subscribe \
		  test_unsubscribe \
		  test_renew
	
   


