#
# CMakeLists.txt for openwsman/src/plugins/swig/ruby
#

enable_testing()
ADD_SUBDIRECTORY(tests)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['vendorarchdir']" OUTPUT_VARIABLE RUBY_ARCH_DIR)
EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['vendorlibdir']" OUTPUT_VARIABLE RUBY_LIB_DIR)

IF( "${RUBY_ARCH_DIR}" STREQUAL "nil" OR "${RUBY_LIB_DIR}" STREQUAL "nil")
  MESSAGE(STATUS "Ruby too old, no vendor_dir, going for site_dir")
  EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['sitearchdir']" OUTPUT_VARIABLE RUBY_ARCH_DIR)
  EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['sitelibdir']" OUTPUT_VARIABLE RUBY_LIB_DIR)
ENDIF( "${RUBY_ARCH_DIR}" STREQUAL "nil" OR "${RUBY_LIB_DIR}" STREQUAL "nil")

MESSAGE(STATUS "Ruby executable: ${RUBY_EXECUTABLE}")
MESSAGE(STATUS "Ruby arch dir: ${RUBY_ARCH_DIR}")
MESSAGE(STATUS "Ruby lib dir: ${RUBY_LIB_DIR}")
MESSAGE(STATUS "Ruby include path: ${RUBY_INCLUDE_PATH}")

SET( SWIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/swig_wrap.c" )
SET( SWIG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/../plugin.i" )

ADD_CUSTOM_COMMAND (
   OUTPUT  ${SWIG_OUTPUT}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating wrapper code for ruby..."
   COMMAND ${SWIG_EXECUTABLE} -ruby -autorename -I${CMAKE_SOURCE_DIR}/include -I${CMAKE_SOURCE_DIR}/bindings -o ${SWIG_OUTPUT} ${SWIG_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../*.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.c
   DEPENDS ${CMAKE_SOURCE_DIR}/bindings/*.i
)


SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC" )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../src )
# wsman_config.h is generated
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
# all other wsman includes
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/bindings )
INCLUDE_DIRECTORIES( ${RUBY_INCLUDE_PATH} )

ADD_DEFINITIONS(-DTARGET_RUBY)

#
# wsman_ruby_plugin.so
#

SET( NAME wsman_ruby_plugin )
ADD_LIBRARY( ${NAME} SHARED ${SWIG_OUTPUT})
TARGET_LINK_LIBRARIES( ${NAME} ${RUBY_LIBRARY} )

INSTALL(TARGETS ${NAME} LIBRARY DESTINATION ${PACKAGE_PLUGIN_DIR})

#
# rbwsmanplugin.rb: plugin implementation
#
INSTALL(FILES rbwsmanplugin.rb DESTINATION ${RUBY_LIB_DIR})
