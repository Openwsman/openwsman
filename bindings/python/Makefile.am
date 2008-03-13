INCLUDES =  -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/bindings \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" \
	   $(PYTHON_CFLAGS)

LIBS = $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/lib/libwsman_curl_client_transport.la


GENERATED = openwsman_wrap.c \
	    pywsman.py

SWIG_SOURCES = ../openwsman.i ../openwsman.c
_pywsman_la_SOURCES = openwsman_wrap.c \
			 $(SWIG_SOURCES)
all-local:	pywsman.py

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(pythondir)
	@INSTALL@ -m 0644 pywsman.py $(DESTDIR)$(pythondir)

_pywsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

_pywsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

python_LTLIBRARIES = _pywsman.la

$(GENERATED): $(SWIG_SOURCES)
	$(SWIG) -python -I$(top_srcdir)/include -o $@ $<

install-data-hook:
	rm -f $(DESTDIR)${pythondir}/$(python_LTLIBRARIES)

CLEANFILES= $(GENERATED) *.pyc


