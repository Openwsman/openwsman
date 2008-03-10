#
# CMakeLists.txt for openwsman/src/authenticators
#

add_subdirectory(file)

IF( DEFINED ${USE_PAM})
add_subdirectory(pam)
ENDIF( DEFINED ${USE_PAM})
