#
# CMakeLists.txt for openwsman/src/cpp
#

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

SET( wsmaninclude_HEADERS OpenWsmanClient.h Exception.h WsmanClient.h WsmanFilter.h WsmanEPR.h WsmanOptions.h )
SET( wsman_clientpp_LIB_SRCS OpenWsmanClient.cpp WsmanEPR.cpp WsmanFilter.cpp WsmanOptions.cpp )
add_library( ${WSMAN_CLIENTPP_PKG} ${wsman_clientpp_LIB_SRCS})

set_target_properties( ${WSMAN_CLIENTPP_PKG} PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS ${WSMAN_CLIENTPP_PKG} DESTINATION ${LIB_INSTALL_DIR})


install(FILES ${wsmaninclude_HEADERS} DESTINATION ${INCLUDE_DIR}/openwsman/cpp)
