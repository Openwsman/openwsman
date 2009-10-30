#
# Makefile.am for openwsman/bindings/perl
#
SUBDIRS = tests

perlarchdir = @PERLVENDORARCHDIR@
perllibdir = @PERLVENDORLIBDIR@

INCLUDES =  \
           -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/bindings \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" \
	   -I@PERLCOREDIR@
	   
openwsman_la_CFLAGS = @PERLCCFLAGS@ -Wno-unused -Wno-error -Wno-nonnull

LDADD = \
       @PERLLDFLAGS@

LIBS = \
       $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/lib/libwsman_curl_client_transport.la


GENERATED = openwsman_wrap.c \
	    openwsman.pm

SWIG_INPUT = ../openwsman.i
openwsman_la_SOURCES = openwsman_wrap.c \
			 ../openwsman.c \
			 $(SWIG_INPUT)

openwsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

openwsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

perlarch_LTLIBRARIES = openwsman.la

$(GENERATED): $(SWIG_INPUT)
	$(SWIG) -perl -I$(top_srcdir)/include -I$(top_srcdir)/bindings -o $@ $<

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(perllibdir)
	@INSTALL@ -m 0644 openwsman.pm $(DESTDIR)$(perllibdir)

install-data-hook:
	rm -f $(DESTDIR)$(perlarchdir)/$(perlarch_LTLIBRARIES)

CLEANFILES= $(GENERATED)

dist-hook:
	rm -f $(distdir)/openwsman_wrap.c
	rm -f $(distdir)/openwsman.pm

EXTRA_DIST = helpers.c
