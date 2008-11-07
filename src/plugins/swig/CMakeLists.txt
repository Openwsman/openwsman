#
# CMakeLists.txt for openwsman/src/plugins/swig
#

enable_testing()

IF ( NOT SWIG_EXECUTABLE )
  MESSAGE( FATAL "SWIG not found, can't build swig plugins." )
ENDIF ( NOT SWIG_EXECUTABLE )

IF(BUILD_RUBY)
add_subdirectory(ruby)
ENDIF(BUILD_RUBY)

IF(BUILD_PYTHON)
#add_subdirectory(python)
ENDIF(BUILD_PYTHON)

IF(BUILD_PERL)
#add_subdirectory(perl)
ENDIF(BUILD_PERL)
