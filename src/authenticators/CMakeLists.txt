#
# CMakeLists.txt for openwsman/src/authenticators
#

include_directories(${CMAKE_BINARY_DIR})
add_subdirectory(file)

IF( USE_PAM )
add_subdirectory(pam)
ENDIF( USE_PAM )
