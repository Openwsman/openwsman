#
# CMakeLists.txt for openwsman/src/lib/test
#

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

SET(test_list_SOURCES test_list.c)
SET(test_string_SOURCES test_string.c)
SET(test_md5_SOURCES test_md5.c)
ADD_EXECUTABLE(test_list ${test_list_SOURCES})
ADD_EXECUTABLE(test_string ${test_string_SOURCES})
ADD_EXECUTABLE(test_md5 ${test_md5_SOURCES})

SET( TEST_LIBS wsman wsman_client ${LIBXML2_LIBRARIES} ${CURL_LIBRARIES} "pthread")
TARGET_LINK_LIBRARIES( test_list ${TEST_LIBS} )
TARGET_LINK_LIBRARIES( test_string ${TEST_LIBS} )
TARGET_LINK_LIBRARIES( test_md5 ${TEST_LIBS} )
