#
# CMakeLists.txt for openwsman/src/authenticators
#

add_subdirectory(file)

IF( USE_PAM )
add_subdirectory(pam)
ENDIF( USE_PAM )
