javadir = @libdir@/java

INCLUDES =  -I$(top_srcdir) \
	   -I$(top_srcdir)/include \
	   -I$(top_srcdir)/bindings \
	    $(OPENSSL_CFLAGS) \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\" 

LIBS = $(OPENSSL_LIBS) \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_client.la \
       $(top_builddir)/src/lib/libwsman_curl_client_transport.la


GENERATED = openwsman_wrap.c 

SWIG_SOURCES = ../openwsman.i ../openwsman.c
libjwsman_la_SOURCES = openwsman_wrap.c \
			 $(SWIG_SOURCES)

libjwsman_la_LIBADD = $(LIBS) -lpthread $(CURL_LIBS)

libjwsman_la_LDFLAGS = -module -avoid-version $(NO_UNDEFINED)

java_LTLIBRARIES = libjwsman.la

$(GENERATED): $(SWIG_SOURCES)
	$(SWIG) -java -I$(top_srcdir)/include -o $@ $<

CLEANFILES= $(GENERATED) *.java *.class


