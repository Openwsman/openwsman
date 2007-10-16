

pylibdir = $(shell python -c 'import sys; print sys.path[2]')
pythondir = $(pylibdir)/site-packages


INCLUDES =  \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" \
	   $(PYTHON_CFLAGS)

LIBS = \
       $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/lib/libwsman_curl_client_transport.la


GENERATED = openwsman_wrap.c \
	    pywsman.py
SWIG_SOURCES = openwsman.i
_pywsman_la_SOURCES = openwsman_wrap.c \
			 openwsman.c \
			 $(SWIG_SOURCES)


_pywsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

_pywsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

python_LTLIBRARIES = _pywsman.la

openwsman_wrap.c: $(SWIG_SOURCES)
	$(SWIG) -python -o $@ $<

install-data-local:
	$(mkinstalldirs) ${pythondir}
	@INSTALL@ -m 0644 pywsman.py ${pythondir}
	@INSTALL@ -m 0644 OWsman.py ${pythondir}

CLEANFILES= $(GENERATED)

