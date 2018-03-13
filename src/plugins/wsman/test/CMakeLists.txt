#
# CMakeLists.txt for openwsman/src/plugins/wsman/test
#

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )
SET(wsman_test_SOURCES wsman_test.c wsman_test_stubs.c wsman_test.h)

ADD_LIBRARY( wsman_test ${wsman_test_SOURCES} )

SET_TARGET_PROPERTIES(wsman_test PROPERTIES VERSION 1.0.0 SOVERSION 1)
INSTALL(TARGETS wsman_test DESTINATION ${PACKAGE_PLUGIN_DIR})
