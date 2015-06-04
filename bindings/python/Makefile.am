#
# Makefile.am for openwsman/bindings/python
#
SUBDIRS = tests

AM_CFLAGS =  -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/bindings \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" \
	   $(PYTHON_CFLAGS)

LIBS = $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/lib/libwsman_curl_client_transport.la

GENERATED = openwsman_wrap.c

SWIG_INPUT = ../openwsman.i
_pywsman_la_SOURCES = openwsman_wrap.c \
			 ../openwsman.c \
			 $(SWIG_INPUT)

all-local: $(GENERATED)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(pythondir)
	@INSTALL@ -m 0644 pywsman.py $(DESTDIR)$(pythondir)

_pywsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

_pywsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

pyexec_LTLIBRARIES = _pywsman.la

$(GENERATED): $(SWIG_INPUT)
	$(SWIG) -python -I$(top_srcdir)/include -I$(top_srcdir)/bindings -o $@ $<

install-data-hook:
	rm -f $(DESTDIR)${pyexecdir}/$(pyexec_LTLIBRARIES)

CLEANFILES= $(GENERATED) pywsman.py *.pyc

dist-hook:
	rm -f $(distdir)/openwsman_wrap.c
	rm -f $(distdir)/pywsman.py

EXTRA_DIST=helpers.h
