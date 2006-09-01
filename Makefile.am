SUBDIRS = src doc etc

package: dist
	cp openwsman.spec /usr/src/packages/SPEC
	cp openwsman-*.tar.gz /usr/src/packages/SOURCES
	rpmbuild -ba openwsman.spec
