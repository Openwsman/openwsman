include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#
#plugindir = @PACKAGE_PLUGIN_DIR@
#
#libwsman_identify_plugin_la_SOURCES = \
#	identify.c \
#	identify_stubs.c \
#	identify.h
#
#INCLUDES = -I$(top_srcdir)/include
#
#LIBS = \
#    $(XML_LIBS) 
#
#plugin_LTLIBRARIES= libwsman_identify_plugin.la
#
#libwsman_identify_plugin_la_LIBADD =       \
#        $(XML_LIBS)   \
#	-lpthread
#
#libwsman_identify_plugin_la_LDFLAGS= -version-info 1:0
#
