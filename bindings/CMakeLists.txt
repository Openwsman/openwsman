#
# CMakeLists.txt for openwsman/bindings
#
include_directories(${CMAKE_BINARY_DIR})

IF( BUILD_PYTHON )
add_subdirectory(python)
ENDIF( BUILD_PYTHON )

IF( BUILD_RUBY )
add_subdirectory(ruby)
ENDIF( BUILD_RUBY )

IF( BUILD_JAVA )
add_subdirectory(java)
ENDIF( BUILD_JAVA )

IF( BUILD_PERL )
add_subdirectory(perl)
ENDIF( BUILD_PERL )
