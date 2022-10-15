#
# CMakeLists.txt for openwsman/tests/epr
#

ENABLE_TESTING()

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET( TEST_LIBS wsman wsman_client ${LIBXML2_LIBRARIES} ${CURL_LIBRARIES} "pthread" cunit)

SET( ser1_SOURCES ser1.c )

ADD_EXECUTABLE( ser1 ${ser1_SOURCES} )

TARGET_LINK_LIBRARIES( ser1 ${TEST_LIBS} )

ADD_TEST( ser1 ser1 )