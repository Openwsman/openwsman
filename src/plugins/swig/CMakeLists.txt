#
# CMakeLists.txt for openwsman/src/plugins/swig
#

#FIND_PACKAGE(SWIG REQUIRED)
MESSAGE(STATUS "Looking modules in ${CMAKE_MODULE_PATH}")
FIND_PROGRAM(SWIG_EXECUTABLE
  NAMES swig-1.3 swig
  PATHS ${SWIG_DIR} ${SWIG_DIR}/.. ${SWIG_DIR}/../../bin /usr/bin /usr/local/bin ${CMAKE_INSTALL_PREFIX}/bin
)

IF ( NOT SWIG_EXECUTABLE )
  MESSAGE( FATAL "SWIG not found." )
ELSE ( NOT SWIG_EXECUTABLE )
  MESSAGE( STATUS "SWIG found at ${SWIG_EXECUTABLE}" )
ENDIF ( NOT SWIG_EXECUTABLE )

IF(BUILD_RUBY)
add_subdirectory(ruby)
ENDIF(BUILD_RUBY)

IF(BUILD_PYTHON)
#add_subdirectory(python)
ENDIF(BUILD_PYTHON)
