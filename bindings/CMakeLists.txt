#
# CMakeLists.txt for openwsman/bindings
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.i.in ${CMAKE_CURRENT_BINARY_DIR}/version.i)

SET( SWIG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/openwsman.i" )

include_directories(${CMAKE_BINARY_DIR})

IF( BUILD_PYTHON3 OR BUILD_PYTHON )
add_subdirectory(python)
ENDIF( BUILD_PYTHON3 OR BUILD_PYTHON )

IF( BUILD_RUBY )
add_subdirectory(ruby)
ENDIF( BUILD_RUBY )

IF( BUILD_JAVA )
add_subdirectory(java)
ENDIF( BUILD_JAVA )

IF( BUILD_PERL )
add_subdirectory(perl)
ENDIF( BUILD_PERL )
