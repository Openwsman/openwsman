#
# Makefile.am for openwsman/bindings/ruby/openwsman
#

rubylibdir = $(shell ruby -r rbconfig -e "vld = Config::CONFIG['vendorlibdir']; print(vld ? vld : Config::CONFIG['sitelibdir'])")

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(rubylibdir)/openwsman
	@INSTALL@ -m 0644 openwsman.rb $(DESTDIR)$(rubylibdir)/openwsman
	@INSTALL@ -m 0644 xmlnode.rb $(DESTDIR)$(rubylibdir)/openwsman

EXTRA_DIST= openwsman.rb xmlnode.rb
