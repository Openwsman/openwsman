#
# CMakeLists.txt for openwsman/src/plugins/redirect
#


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )
SET(redirect_SOURCES redirect.c redirect.h redirect_stubs.c)

ADD_LIBRARY( redirect ${redirect_SOURCES} )

SET_TARGET_PROPERTIES(redirect PROPERTIES VERSION 1.0.0 SOVERSION 1)
INSTALL(TARGETS redirect DESTINATION ${PACKAGE_PLUGIN_DIR})
