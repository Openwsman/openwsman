#
# CMakeLists.txt for openwsman/bindings/java
#
#
# This creates several files
#  1. libjwsman.so
#  2. *.java
#

MESSAGE(STATUS "Java compiler: ${JAVA_COMPILE}")
MESSAGE(STATUS "Java runtime: ${JAVA_RUNTIME}")
MESSAGE(STATUS "Java archiver: ${JAVA_ARCHIVE}")
MESSAGE(STATUS "Java include path: ${JAVA_INCLUDE_PATH}")

SET( SWIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/openwsman_wrap.c" )
SET( SWIG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/../openwsman.i" )

ADD_CUSTOM_COMMAND (
   OUTPUT  ${SWIG_OUTPUT}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating wrapper code for Java ..."
   COMMAND ${SWIG_EXECUTABLE} -java -package org.openwsman -features autodoc -I${CMAKE_SOURCE_DIR}/include -o ${SWIG_OUTPUT} ${SWIG_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../*.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

SET( jar_NAME "openwsman-${PACKAGE_ARCHITECTURE}-${VERSION}.jar" )

ADD_CUSTOM_COMMAND (
   OUTPUT ${jar_NAME}
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E echo_append "Compiling Java files ..."
   COMMAND ${JAVA_COMPILE} -d . *.java
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating JAR ..."
   COMMAND ${JAVA_ARCHIVE} cvf ${jar_NAME} *.so org/*
   DEPENDS ${SWIG_OUTPUT}
)

ADD_CUSTOM_TARGET ( jwsman_all ALL
   COMMAND ${CMAKE_COMMAND} -E echo_append "Building Java bindings ..."
   DEPENDS ${jar_NAME}
)

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )

SET(jwsman_SRCS ${SWIG_OUTPUT} ${CMAKE_CURRENT_SOURCE_DIR}/../openwsman.c )
ADD_LIBRARY( jwsman SHARED ${jwsman_SRCS} )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/bindings )
INCLUDE_DIRECTORIES( ${JNI_INCLUDE_DIRS} )

TARGET_LINK_LIBRARIES( jwsman wsman )
TARGET_LINK_LIBRARIES( jwsman wsman_client )

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${jar_NAME} DESTINATION /usr/share/java)

enable_testing()

add_subdirectory(tests)
