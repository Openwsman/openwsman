include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )


########### next target ###############

SET(wsman_clientpp_LIB_SRCS
   \
)

kde3_automoc(${wsman_clientpp_LIB_SRCS})

add_library(wsman_clientpp SHARED ${wsman_clientpp_LIB_SRCS})

target_link_libraries(wsman_clientpp ${QT_AND_KDECORE_LIBS})

set_target_properties(wsman_clientpp PROPERTIES VERSION 4.2.0 SOVERSION 4)
install(TARGETS wsman_clientpp DESTINATION lib)


########### install files ###############

install(FILES  \ DESTINATION /include/openwsman/cpp)



#original Makefile.am contents follow:

#
#wsmanincludedir = $(includedir)/openwsman/cpp
#
#INCLUDES =  \
#	   -I$(top_srcdir) \
#	   -I$(top_srcdir)/include \
#	    $(OPENSSL_CFLAGS) \
#	   -DSYSCONFDIR=\""$(sysconfdir)/openwsman"\"
#
#LIBS = \
#    $(XML_LIBS)  \
#    $(top_builddir)/src/lib/libwsman.la \
#    $(top_builddir)/src/lib/libwsman_client.la 
#
#wsmaninclude_HEADERS = \
#		       OpenWsmanClient.h  \
#		       Exception.h \
#			WsmanClient.h
#
#lib_LTLIBRARIES=libwsman_clientpp.la
#
#libwsman_clientpp_la_SOURCES = \
#	OpenWsmanClient.cpp  \
#	OpenWsmanClient.h \
#	Exception.h \
#	WsmanClient.h
#
#
#libwsman_clientpp_la_CPPFLAGS = 
#
#
#
#
