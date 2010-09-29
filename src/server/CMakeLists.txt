#
# CMakeLists.txt for openwsman/src/server
#
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/server/shttpd ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g" )

SET(openwsmand_SOURCES shttpd/string.c shttpd/shttpd.c shttpd/auth.c shttpd/md5.c shttpd/adapter.c shttpd/cgi.c shttpd/mime_type.c shttpd/config.c shttpd/io_socket.c shttpd/io_ssl.c shttpd/io_emb.c shttpd/compat_unix.c shttpd/io_dir.c shttpd/io_file.c shttpd/llist.h shttpd/shttpd.h shttpd/std_includes.h shttpd/io.h shttpd/md5.h shttpd/ssl.h shttpd/compat_unix.h shttpd/compat_win32.h shttpd/compat_rtems.h shttpd/adapter.h wsmand-listener.h wsmand-daemon.c wsmand-daemon.h wsmand-listener.c wsmand.c)

ADD_DEFINITIONS(-DEMBEDDED -DNO_CGI -DNO_SSI )
ADD_DEFINITIONS(-DPACKAGE_PLUGIN_DIR="\\\"${PACKAGE_PLUGIN_DIR}\\\"")
ADD_DEFINITIONS(-DPACKAGE_AUTH_DIR="\\\"${PACKAGE_AUTH_DIR}\\\"")
ADD_DEFINITIONS(-DPACKAGE_SUBSCRIPTION_DIR="\\\"${PACKAGE_SUBSCRIPTION_DIR}\\\"")

ADD_EXECUTABLE(openwsmand ${openwsmand_SOURCES})
TARGET_LINK_LIBRARIES(openwsmand wsman_server wsman ${LIBXML2_LIBRARIES})

if( ENABLE_EVENTING_SUPPORT )
TARGET_LINK_LIBRARIES(openwsmand wsman_client)
TARGET_LINK_LIBRARIES(openwsmand ${CURL_LIBRARIES})
endif( ENABLE_EVENTING_SUPPORT )

if( USE_OPENSSL )
include_directories(${OPENSSL_INCLUDE_DIR})
ADD_DEFINITIONS(-DHAVE_OPENSSL )
TARGET_LINK_LIBRARIES(openwsmand ${OPENSSL_LIBRARIES})
endif( USE_OPENSSL )

if( USE_PTHREAD )
TARGET_LINK_LIBRARIES(openwsmand ${CMAKE_THREAD_LIBS_INIT})
endif( USE_PTHREAD )

if( HAVE_LIBDL )
TARGET_LINK_LIBRARIES(openwsmand ${DL_LIBRARIES})
endif( HAVE_LIBDL )

INSTALL(TARGETS openwsmand DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin)

#		
#
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(PACKAGE_SUBSCRIPTION_DIR)
