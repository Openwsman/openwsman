AM_CFLAGS = @CFLAGS@

INCLUDES = \
	   $(OPENWSMAN_CFLAGS) \
       $(CUNIT_CFLAGS)

	
wsmantests_LDADD = \
       $(OPENWSMAN_LIBS) $(CUNIT_LIBS) -lcurl


wsmantests_SOURCES = run_tests.c \
		    common.c \
		    common.h \
		    identify.c \
		    enumeration.c \
		    transfer_get.c

noinst_PROGRAMS = wsmantests 

