#
# CMakeLists.txt for openwsman/tests/epr
#

ENABLE_TESTING()

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET( TEST_LIBS wsman wsman_client ${LIBXML2_LIBRARIES} ${CURL_LIBRARIES} "pthread")

SET( test_epr_SOURCES test_epr.c )

ADD_EXECUTABLE( test_epr ${test_epr_SOURCES} )

TARGET_LINK_LIBRARIES( test_epr ${TEST_LIBS} )

ADD_TEST(test_epr test_epr)