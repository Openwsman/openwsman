AM_CFLAGS = @CFLAGS@ -DPACKAGE_PLUGIN_DIR="\"${PACKAGE_PLUGIN_DIR}\""

INCLUDES = \
	   $(GMODULE_CFLAGS) \
	   $(SOUP_CFLAGS) \
	   -I$(top_srcdir) \
	   -I$(top_srcdir)/src/lib \
	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\"

LIBS = \
       $(GMODULE_LIBS) \
       $(SOUP_LIBS) \
       -lcrypt \
       $(top_builddir)/src/lib/libwsman.la \
       $(top_builddir)/src/lib/libwsman_utils.la \
       $(top_builddir)/src/lib/libwsman_server.la


sbin_PROGRAMS = wsmand

wsmand_SOURCES = \
		 wsmand-listener.h \
		 wsmand-listener.c \
		 wsmand-daemon.c \
		 wsmand-daemon.h \
		 wsmand-auth.c \
		 wsmand-auth.h \
		 wsmand.c 

#wsmand_LDADD =     \
#		$(LINK_BEGIN) $(GMODULE_LIBS) $(SOUP_LIBS) $(LINK_END) $(SHARED_ONLY)


#wsmand: $(wsmand_OBJECTS) $(wsmand_DEPENDENCIES)
#	@rm -f wsmand
#	$(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(wsmand_LDFLAGS) $(wsmand_OBJECTS) $(wsmand_LDADD) $(LIBS)	       
