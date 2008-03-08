

#pylibdir = $(shell python -c 'import sys; print sys.path[2]')
#pythondir = $(pylibdir)/site-packages

# SWIG got new options a 1.3.32
# if SWIG_NEW_OPTIONS
# swigoptions = -debug-classes -debug-module 1,2,3,4
# else
# swigoptions = -dump_classes -dump_module -dump_parse_module
# endif

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

SWIG_SOURCES = ../openwsman.i
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

CLEANFILES= $(GENERATED)


