#
# CMakeLists.txt for openwsman/src/authenticators/pam
#


include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET( pam_auth_SOURCES pam_auth.c )

ADD_LIBRARY( wsman_pam_auth ${pam_auth_SOURCES} )
TARGET_LINK_LIBRARIES( wsman_pam_auth ${CRYPT_LIBRARIES} )
TARGET_LINK_LIBRARIES( wsman_pam_auth ${HAVE_LIBPAM} )
if( HAVE_LIBDL )
TARGET_LINK_LIBRARIES( wsman_pam_auth ${DL_LIBRARIES})
endif( HAVE_LIBDL )

SET_TARGET_PROPERTIES( wsman_pam_auth PROPERTIES VERSION 1.0.0 SOVERSION 1)

INSTALL(TARGETS wsman_pam_auth DESTINATION ${PACKAGE_AUTH_DIR})