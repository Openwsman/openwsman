#
# CMakeLists.txt for openwsman/examples
#

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# serialize

SET( serialize_SOURCES serialize.c )
ADD_EXECUTABLE( serialize ${serialize_SOURCES} )

SET( TEST_LIBS wsman wsman_client ${LIBXML2_LIBRARIES} ${CURL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
TARGET_LINK_LIBRARIES( serialize ${TEST_LIBS} )

# test.xml

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/test.xml ${CMAKE_CURRENT_BINARY_DIR}/test.xml COPYONLY )

# winrs.rb

INSTALL(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/winrs.rb DESTINATION ${CMAKE_INSTALL_PREFIX}/bin RENAME winrs)
