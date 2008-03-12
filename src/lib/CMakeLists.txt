#
# CMakeLists.txt for openwsman/src/lib
#

ADD_SUBDIRECTORY(test)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

ADD_DEFINITIONS( -DPACKAGE_PLUGIN_DIR="\\\"${PACKAGE_PLUGIN_DIR}\\\"" )

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${XML_CFLAGS} -g" )


########### wsman ###############


SET( UTIL_SOURCES u/buf.c u/log.c u/memory.c u/misc.c  u/uri.c  u/uuid.c u/lock.c u/md5.c u/strings.c u/list.c u/hash.c u/base64.c u/iniparser.c u/debug.c u/uerr.c u/uoption.c u/gettimeofday.c u/syslog.c u/pthreadx_win32.c u/os.c )

SET( wsman_SOURCES ${UTIL_SOURCES} wsman-libxml2-binding.c wsman-xml.c wsman-epr.c wsman-filter.c wsman-dispatcher.c wsman-soap.c wsman-faults.c wsman-xml-serialize.c wsman-soap-envelope.c wsman-debug.c wsman-soap-message.c )

IF( ENABLE_EVENTING_SUPPORT )
SET( wsman_SOURCES ${wsman_SOURCES} wsman-subscription-repository.c wsman-event-pool.c wsman-cimindication-processor.c )
ENDIF( ENABLE_EVENTING_SUPPORT )

ADD_LIBRARY( wsman SHARED ${wsman_SOURCES} )
TARGET_LINK_LIBRARIES( wsman ${LIBXML2_LIBRARIES} )
TARGET_LINK_LIBRARIES( wsman ${CMAKE_THREAD_LIBS_INIT} )

IF( ENABLE_EVENTING_SUPPORT )
TARGET_LINK_LIBRARIES( wsman wsman_client )
ENDIF( ENABLE_EVENTING_SUPPORT )

SET_TARGET_PROPERTIES(wsman PROPERTIES VERSION 1.0.0 SOVERSION 1)
INSTALL(TARGETS wsman DESTINATION lib)

########### wsman_server ###############

IF ( NOT DISABLE_SERVER )
 SET( wsman_server_SOURCES wsman-server.c wsman-plugins.c wsman-server-api.c )
 ADD_LIBRARY( wsman_server SHARED ${wsman_server_SOURCES} )
 TARGET_LINK_LIBRARIES( wsman_server wsman )
 SET_TARGET_PROPERTIES(wsman_server PROPERTIES VERSION 1.0.0 SOVERSION 1)
 INSTALL(TARGETS wsman_server DESTINATION lib)
ENDIF ( NOT DISABLE_SERVER )

########## wsman_curl_client_transport ###############

SET( wsman_curl_client_transport_SOURCES wsman-client-transport.c wsman-curl-client-transport.c )
ADD_LIBRARY( wsman_curl_client_transport SHARED ${wsman_curl_client_transport_SOURCES} )
TARGET_LINK_LIBRARIES( wsman_curl_client_transport ${CURL_LIBRARIES} )
SET_TARGET_PROPERTIES(wsman_curl_client_transport PROPERTIES VERSION 1.0.0 SOVERSION 1)
INSTALL(TARGETS wsman_curl_client_transport DESTINATION lib)


############### wsman_client ###############


SET( wsman_client_SOURCES wsman-client.c )
ADD_LIBRARY( wsman_client SHARED ${wsman_client_SOURCES} )
TARGET_LINK_LIBRARIES( wsman_client wsman_curl_client_transport )
SET_TARGET_PROPERTIES(wsman_client PROPERTIES VERSION 1.0.0 SOVERSION 1)
INSTALL(TARGETS wsman_client DESTINATION lib)

#
#EXTRA_DIST = \
#	wsman-win-client-transport.c
