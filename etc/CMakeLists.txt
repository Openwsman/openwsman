add_subdirectory(init)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### install files ###############

install(FILES  owsmangencert.sh DESTINATION $(sysconfdir)/openwsman )
install(FILES  ssleay.cnf openwsman.conf DESTINATION $(sysconfdir)/openwsman )

#original Makefile.am contents follow:

#SUBDIRS =  init
#
#noinst_DATA = ssleay.cnf openwsman.conf
#openwsmanconf_DATA = owsmangencert.sh
#openwsmanconfdir = $(sysconfdir)/openwsman
#
#EXTRA_DIST = openwsman.conf owsmangencert.sh ssleay.cnf openwsman
#CLEANFILES = owsmangencert
#
