#
# CMakeLists.txt for openwsman/tests/filter
#

ENABLE_TESTING()

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET( TEST_LIBS wsman wsman_client ${LIBXML2_LIBRARIES} ${CURL_LIBRARIES} "pthread")

SET( test_filter_SOURCES test_filter.c )

ADD_EXECUTABLE( test_filter ${test_filter_SOURCES} )

TARGET_LINK_LIBRARIES( test_filter ${TEST_LIBS} )

ADD_TEST( test_filter test_filter )