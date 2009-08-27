
rubydir = $(shell ruby -r rbconfig -e "print Config::CONFIG['sitearchdir']")

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


GENERATED = openwsman_wrap.c \
	    openwsman.rb
SWIG_SOURCES = ../openwsman.i ../openwsman.c
openwsman_la_SOURCES = openwsman_wrap.c \
			 $(SWIG_SOURCES)


openwsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

openwsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

ruby_LTLIBRARIES = openwsman.la

openwsman_wrap.c: $(SWIG_SOURCES)
	$(SWIG) -ruby -I$(top_srcdir)/include -I$(top_srcdir)/bindings -o $@ $<

install-data-hook:
	rm -f $(DESTDIR)${rubydir}/$(ruby_LTLIBRARIES)

CLEANFILES= $(GENERATED)

EXTRA_DIST=helpers.c
