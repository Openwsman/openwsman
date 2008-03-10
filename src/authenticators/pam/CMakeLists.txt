find_package(KDE3 REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(${QT_DEFINITIONS} ${KDE3_DEFINITIONS})

link_directories(${KDE3_LIB_DIR})

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#
##if USE_PAM
#authdir = @PACKAGE_AUTH_DIR@
#
#auth_LTLIBRARIES= libwsman_pam_auth.la
#libwsman_pam_auth_la_SOURCES = \
#	pam_auth.c
#
#INCLUDES = -I$(top_srcdir)/include
#
#libwsman_pam_auth_la_LIBADD = -lpam
#
#libwsman_pam_auth_la_LDFLAGS = -version-info 1:0
#
##endif
