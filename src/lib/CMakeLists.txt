#
# CMakeLists.txt for openwsman/src/lib
#

IF ( BUILD_TESTS )
ADD_SUBDIRECTORY(test)
ENDIF ( BUILD_TESTS )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

ADD_DEFINITIONS( -DPACKAGE_PLUGIN_DIR="${PACKAGE_PLUGIN_DIR}" )

if (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${XML_CFLAGS} -DIGNORE_DUPLICATE_ID -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE" )
else (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${XML_CFLAGS} -g -DIGNORE_DUPLICATE_ID" )
endif (CMAKE_CXX_COMPILER_ID MATCHES MSVC)


########### wsman ###############


SET( UTIL_SOURCES u/buf.c u/log.c u/memory.c u/misc.c  u/uri.c  u/uuid.c u/lock.c u/md5.c u/strings.c u/list.c u/hash.c u/base64.c u/iniparser.c u/debug.c u/uerr.c u/uoption.c u/gettimeofday.c u/syslog.c u/pthreadx_win32.c u/os.c )

SET( wsman_SOURCES ${UTIL_SOURCES} wsman-libxml2-binding.c wsman-xml.c wsman-epr.c wsman-key-value.c wsman-filter.c wsman-dispatcher.c wsman-soap.c wsman-faults.c wsman-xml-serialize.c wsman-soap-envelope.c wsman-debug.c wsman-soap-message.c)

IF( ENABLE_EVENTING_SUPPORT )
SET( wsman_SOURCES ${wsman_SOURCES} wsman-subscription-repository.c wsman-event-pool.c wsman-cimindication-processor.c )
ENDIF( ENABLE_EVENTING_SUPPORT )

ADD_LIBRARY( wsman ${wsman_SOURCES} )
TARGET_LINK_LIBRARIES( wsman ${LIBXML2_LIBRARIES} )
ADD_DEFINITIONS( ${LIBXML2_DEFINITIONS} )
TARGET_LINK_LIBRARIES( wsman ${CMAKE_THREAD_LIBS_INIT} )
if( HAVE_LIBDL )
TARGET_LINK_LIBRARIES(wsman ${DL_LIBRARIES})
endif( HAVE_LIBDL )

IF( ENABLE_EVENTING_SUPPORT )
TARGET_LINK_LIBRARIES( wsman ${WSMAN_CLIENT_PKG} )
ENDIF( ENABLE_EVENTING_SUPPORT )

SET_TARGET_PROPERTIES(wsman PROPERTIES VERSION 1.0.0 SOVERSION 1)
INSTALL(TARGETS wsman DESTINATION ${LIB_INSTALL_DIR})

########### wsman_server ###############

IF ( NOT DISABLE_SERVER )
 SET( wsman_server_SOURCES wsman-server.c wsman-plugins.c wsman-server-api.c )
 ADD_LIBRARY( ${WSMAN_SERVER_PKG} ${wsman_server_SOURCES} )
 TARGET_LINK_LIBRARIES( ${WSMAN_SERVER_PKG} wsman )
 SET_TARGET_PROPERTIES( ${WSMAN_SERVER_PKG} PROPERTIES VERSION 1.0.0 SOVERSION 1)
 INSTALL(TARGETS ${WSMAN_SERVER_PKG} DESTINATION ${LIB_INSTALL_DIR})
ENDIF ( NOT DISABLE_SERVER )

########## wsman_curl_client_transport ###############
IF(UNIX)
 SET( wsman_curl_client_transport_SOURCES wsman-client-transport.c wsman-curl-client-transport.c )
 ADD_LIBRARY( ${WSMAN_CLIENT_TRANSPORT_PKG} ${wsman_curl_client_transport_SOURCES} )
 TARGET_LINK_LIBRARIES( ${WSMAN_CLIENT_TRANSPORT_PKG} ${CURL_LIBRARIES} )
ELSE(UNIX)
 SET( wsman_win_client_transport_SOURCES wsman-client-transport.c wsman-win-client-transport.c )
 ADD_LIBRARY( ${WSMAN_CLIENT_TRANSPORT_PKG} ${wsman_win_client_transport_SOURCES} )
ENDIF(UNIX)
IF( ENABLE_EVENTING_SUPPORT )
TARGET_LINK_LIBRARIES( ${WSMAN_CLIENT_TRANSPORT_PKG} ${OPENSSL_LIBRARIES} )
ENDIF( ENABLE_EVENTING_SUPPORT )
SET_TARGET_PROPERTIES( ${WSMAN_CLIENT_TRANSPORT_PKG} PROPERTIES VERSION 1.0.0 SOVERSION 1)
INSTALL(TARGETS ${WSMAN_CLIENT_TRANSPORT_PKG} DESTINATION ${LIB_INSTALL_DIR})


############### wsman_client ###############

SET( wsman_client_SOURCES wsman-client.c )
ADD_LIBRARY( ${WSMAN_CLIENT_PKG} ${wsman_client_SOURCES} )
TARGET_LINK_LIBRARIES( ${WSMAN_CLIENT_PKG} ${WSMAN_CLIENT_TRANSPORT_PKG} )
SET_TARGET_PROPERTIES( ${WSMAN_CLIENT_PKG} PROPERTIES VERSION 5.0.0 SOVERSION 5)
INSTALL(TARGETS ${WSMAN_CLIENT_PKG} DESTINATION ${LIB_INSTALL_DIR})

#
#EXTRA_DIST = \
#	wsman-win-client-transport.c
