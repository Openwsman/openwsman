#
# CMakeLists.txt for openwsman/tests/client
#

ENABLE_TESTING()

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET( TEST_LIBS wsman wsman_client ${LIBXML2_LIBRARIES} ${CURL_LIBRARIES} "pthread")

SET( xml1_SOURCES xml1.c )
SET( xml2_SOURCES xml2.c )
SET( xml3_SOURCES xml3.c )
SET( xml4_SOURCES xml4.c )

ADD_EXECUTABLE( xml1 ${xml1_SOURCES} )
ADD_EXECUTABLE( xml2 ${xml2_SOURCES} )
ADD_EXECUTABLE( xml3 ${xml3_SOURCES} )
ADD_EXECUTABLE( xml4 ${xml4_SOURCES} )

TARGET_LINK_LIBRARIES( xml1 ${TEST_LIBS} )
TARGET_LINK_LIBRARIES( xml2 ${TEST_LIBS} )
TARGET_LINK_LIBRARIES( xml3 ${TEST_LIBS} )
TARGET_LINK_LIBRARIES( xml4 ${TEST_LIBS} )

ADD_TEST( xml1 xml1 ${CMAKE_CURRENT_SOURCE_DIR}/cim_computersystem_01.xml )
ADD_TEST( xml2 xml2 )
ADD_TEST( xml3 xml3 )
ADD_TEST( xml4 xml4 ${CMAKE_CURRENT_SOURCE_DIR}/cim_computersystem_02.xml )
