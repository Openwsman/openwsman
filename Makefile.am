SUBDIRS = src doc etc tests

wsmanincludedir = $(includedir)/openwsman
wsmaninclude_HEADERS = wsman_config.h

package: dist
	cp openwsman.spec /usr/src/packages/SPEC
	cp openwsman-*.tar.gz /usr/src/packages/SOURCES
	rpmbuild -ba openwsman.spec

etags: TAGS
	find . -name "*.[chCH]" -print | etags -

install-data-local: openwsman.pc
	$(mkinstalldirs) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) -D openwsman.pc $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) -D openwsman-server.pc $(DESTDIR)$(libdir)/pkgconfig

