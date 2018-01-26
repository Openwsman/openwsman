#
# CMakeLists.txt for openwsman/src/authenticators/file
#

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET( file_auth_SOURCES file_auth.c )

ADD_LIBRARY( wsman_file_auth ${file_auth_SOURCES} )
TARGET_LINK_LIBRARIES( wsman_file_auth ${CRYPT_LIBRARIES} )
SET_TARGET_PROPERTIES( wsman_file_auth PROPERTIES VERSION 1.0.0 SOVERSION 1)

INSTALL(TARGETS wsman_file_auth DESTINATION ${PACKAGE_AUTH_DIR})
