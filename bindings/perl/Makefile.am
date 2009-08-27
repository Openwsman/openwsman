#
# Makefile.am for openwsman/bindings/perl
#

perldir = @PERLVENDORARCHDIR@

INCLUDES =  \
           -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/bindings \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" \
	   -I@PERLCOREDIR@
	   
CFLAGS += @PERLCCFLAGS@ -Wno-unused -Wno-error -Wno-nonnull

LDADD = \
       @PERLLDFLAGS@

LIBS = \
       $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/lib/libwsman_curl_client_transport.la


GENERATED = openwsman_wrap.c \
	    openwsman.pm
SWIG_SOURCES = ../openwsman.i
openwsman_la_SOURCES = openwsman_wrap.c \
			 $(SWIG_SOURCES)


openwsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

openwsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

perl_LTLIBRARIES = openwsman.la

openwsman_wrap.c: $(SWIG_SOURCES)
	$(SWIG) -perl -I$(top_srcdir)/include -I$(top_srcdir)/bindings -o $@ $<

install-data-hook:
	rm -f $(DESTDIR)$(perldir)/$(perl_LTLIBRARIES)

CLEANFILES= $(GENERATED)

EXTRA_DIST = helpers.c
