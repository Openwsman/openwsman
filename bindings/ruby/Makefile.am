#
# Makefile.am for openwsman/bindings/ruby
#

SUBDIRS = openwsman

rubylibdir = $(shell ruby -r rbconfig -e "print (Config::CONFIG['vendorarchdir'] rescue Config::CONFIG['sitearchdir'])")
rubyarchdir = $(shell ruby -r rbconfig -e "print (Config::CONFIG['vendorarchdir'] rescue Config::CONFIG['sitearchdir'])")

INCLUDES =  \
           -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/bindings \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" \
	   -I${rbincdir}

LDADD = \
       ${rbldadd}       

LIBS = \
       $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/lib/libwsman_curl_client_transport.la

GENERATED = openwsman_wrap.c

SWIG_SOURCES = ../openwsman.i ../openwsman.c

openwsman_la_SOURCES = $(SWIG_SOURCES) $(GENERATED)

openwsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

openwsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

rubylib_LTLIBRARIES = openwsman.la

$(GENERATED): $(SWIG_SOURCES)
	$(SWIG) -ruby -I$(top_srcdir)/include -I$(top_srcdir)/bindings -o $@ $<

install-data-hook:
	rm -f $(DESTDIR)${rubyarchdir}/$(rubylib_LTLIBRARIES)

CLEANFILES= $(GENERATED)

EXTRA_DIST=helpers.c
