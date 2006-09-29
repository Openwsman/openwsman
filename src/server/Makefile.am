AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""

INCLUDES =  \
	    $(GOBJECT_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\"
if USE_LIBSOUP_SERV
INCLUDES += $(SOUP_CFLAGS)  \
	    $(GTHREAD_CFLAGS)

endif

LIBS = \
       $(GOBJECT_LIBS) \
       -lcrypt \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/libu/libwsman_utils.la \
       $(top_builddir)/src/lib/libwsman_server.la

if USE_LIBSOUP_SERV
LIBS += $(SOUP_LIBS)  \
       $(GTHREAD_LIBS)
endif



sbin_PROGRAMS = wsmand

wsmand_SOURCES = \
		 wsmand-listener.h \
		 wsmand-daemon.c \
		 wsmand-daemon.h \
                 wsmand-auth.c \
		 wsmand-auth.h \
		 wsmand.c 

if USE_SHTTPD
wsmand_SOURCES += \
                 shttpd.h \
                 shttpd.c \
		 wsmand-shttpd-listener.c

else
wsmand_SOURCES += \
		 wsmand-libsoup-listener.c

endif
		

#wsmand_LDADD =     \
#		$(LINK_BEGIN) $(GMODULE_LIBS) $(SOUP_LIBS) $(LINK_END) $(SHARED_ONLY)


#wsmand: $(wsmand_OBJECTS) $(wsmand_DEPENDENCIES)
#	@rm -f wsmand
#	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(wsmand_LDFLAGS) $(wsmand_OBJECTS) $(wsmand_LDADD) $(LIBS)	       
