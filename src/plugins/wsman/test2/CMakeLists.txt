include_directories(${CMAKE_SOURCE_DIR}/src/lib ${CMAKE_SOURCE_DIR}/src/libu ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#
#plugindir = @PACKAGE_PLUGIN_DIR@
#
#libwsman_test2_la_SOURCES = \
#	wsman_test2.c \
#	wsman_test2_stubs.c \
#	wsman_test2.h
#
#INCLUDES = -I$(top_srcdir)/src/lib \
#	   -I$(top_srcdir)/src/libu 
#
#LIBS = \
#    $(XML_LIBS)
#
#plugin_LTLIBRARIES= libwsman_test2.la
#
#libwsman_test2_la_LIBADD =       \
#        $(XML_LIBS)   \
#	-lpthread
#
#libwsman_test2_la_LDFLAGS= -version-info 1:0
#
