AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""

INCLUDES = \
	   $(SOUP_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
	   -I$(top_srcdir)/src/libu \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\"

LIBS = \
       $(SOUP_LIBS) \
       -lcrypt \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/libu/libwsman_utils.la \
       $(top_builddir)/src/lib/libwsman_server.la


sbin_PROGRAMS = wsmand

wsmand_SOURCES = \
		 wsmand-libsoup-listener.h \
		 wsmand-libsoup-listener.c \
		 wsmand-daemon.c \
		 wsmand-daemon.h \
		 wsmand-libsoup-auth.c \
		 wsmand-libsoup-auth.h \
		 wsmand.c 

#wsmand_LDADD =     \
#		$(LINK_BEGIN) $(GMODULE_LIBS) $(SOUP_LIBS) $(LINK_END) $(SHARED_ONLY)


#wsmand: $(wsmand_OBJECTS) $(wsmand_DEPENDENCIES)
#	@rm -f wsmand
#	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(wsmand_LDFLAGS) $(wsmand_OBJECTS) $(wsmand_LDADD) $(LIBS)	       
