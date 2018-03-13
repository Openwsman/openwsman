#
# CMakeLists.txt for openwsman/src/plugins/identify
#

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

SET(wsman_identify_plugin_SOURCES identify.c identify_stubs.c identify.h)

ADD_LIBRARY( wsman_identify_plugin ${wsman_identify_plugin_SOURCES} )

SET_TARGET_PROPERTIES(wsman_identify_plugin PROPERTIES VERSION 1.0.0 SOVERSION 1)
INSTALL(TARGETS wsman_identify_plugin DESTINATION ${PACKAGE_PLUGIN_DIR})
