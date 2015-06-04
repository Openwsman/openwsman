#
# Makefile.am for openwsman/bindings/ruby
#

SUBDIRS = openwsman tests

rubyarchdir = $(shell ruby -r rbconfig -e "vad = Config::CONFIG['vendorarchdir']; print(vad ? vad : Config::CONFIG['sitearchdir'])")
rubydir = $(shell ruby -r rbconfig -e "vd = Config::CONFIG['vendorlibdir']; print(vd ? vd : Config::CONFIG['sitelibdir'])")
rubyincdir = $(shell ruby -r rbconfig -e "print(Config::CONFIG['archdir'])")

AM_CFLAGS =  \
           -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/bindings \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" \
	   -I${rubyincdir}

LIBS = \
       $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/lib/libwsman_curl_client_transport.la

GENERATED = openwsman_wrap.c

SWIG_INPUT = ../openwsman.i

openwsman_la_SOURCES =  ../openwsman.c $(SWIG_INPUT) $(GENERATED)

openwsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

openwsman_la_CFLAGS = $(RUBY_CFLAGS)

openwsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

rubyarch_LTLIBRARIES = openwsman.la

$(GENERATED): $(SWIG_INPUT)
	$(SWIG) -ruby -I$(top_srcdir)/include -I$(top_srcdir)/bindings -o $@ $<

install-data-hook:
	rm -f $(DESTDIR)${rubyarchdir}/$(rubyarch_LTLIBRARIES)

CLEANFILES= $(GENERATED)

dist-hook:
	rm -f $(distdir)/openwsman_wrap.c

EXTRA_DIST=helpers.h
