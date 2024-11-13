#
# CMakeLists.txt for openwsman/src/plugins/cim
#

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/cim ${SFCC_INCLUDES} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

CHECK_INCLUDE_FILE("CimClientLib/cmci.h" CMCI_FOUND)

IF(NOT CMCI_FOUND)
  MESSAGE(WARNING "CIM client library not found")
ELSE(NOT CMCI_FOUND)
  MESSAGE(STATUS "Building CIM plugin")

  SET(cim_plugin_SOURCES sfcc-interface.c sfcc-interface.h cim_data.c cim_data_stubs.c cim_data.h )
  ADD_LIBRARY( wsman_cim_plugin ${cim_plugin_SOURCES} )
  TARGET_LINK_LIBRARIES( wsman_cim_plugin wsman )
  TARGET_LINK_LIBRARIES( wsman_cim_plugin ${SFCC_LIBRARIES} )

  SET_TARGET_PROPERTIES(wsman_cim_plugin PROPERTIES VERSION 1.0.0 SOVERSION 1)
  INSTALL(TARGETS wsman_cim_plugin DESTINATION ${PACKAGE_PLUGIN_DIR})
ENDIF(NOT CMCI_FOUND)
