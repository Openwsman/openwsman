SUBDIRS = include src doc etc bindings

if BUILD_EXAMPLES
SUBDIRS += examples
endif
if BUILD_TESTS
SUBDIRS += tests
endif

AUTOMAKE_OPTIONS = no-dist-gzip dist-bzip2

wsmanincludedir = $(includedir)/openwsman
#wsmaninclude_HEADERS = wsman_config.h

package: dist
	cp openwsman.spec /usr/src/packages/SPEC
	cp openwsman-*.tar.gz /usr/src/packages/SOURCES
	rpmbuild -ba openwsman.spec

etags: TAGS
	find . -name "*.[chCH]" -print | etags -

install-data-local: openwsman.pc
	$(mkinstalldirs) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA)  openwsman.pc $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA)  openwsman++.pc $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA)  openwsman-server.pc $(DESTDIR)$(libdir)/pkgconfig

uninstall-local:
	rm $(DESTDIR)$(libdir)/pkgconfig/openwsman.pc
	rm $(DESTDIR)$(libdir)/pkgconfig/openwsman++.pc
	rm $(DESTDIR)$(libdir)/pkgconfig/openwsman-server.pc

EXTRA_DIST = OpenWsmanLib.vcproj OpenWsmanLib.sln
