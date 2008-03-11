#
# CMakeLists.txt for openwsman/src
#

add_subdirectory(lib)
add_subdirectory(cpp)

IF ( NOT DEFINED DISABLE_PLUGINS )
add_subdirectory(plugins)
ENDIF ( NOT DEFINED DISABLE_PLUGINS )

IF ( NOT DEFINED DISABLE_SERVER )
add_subdirectory(server)
add_subdirectory(authenticators)
ENDIF ( NOT DEFINED DISABLE_SERVER )
