#
# CMakeLists.txt for openwsman/src
#

enable_testing()

add_subdirectory(lib)
add_subdirectory(cpp)

IF ( NOT DISABLE_PLUGINS )
add_subdirectory(plugins)
ENDIF ( NOT DISABLE_PLUGINS )

IF ( NOT DISABLE_SERVER )
add_subdirectory(server)
add_subdirectory(authenticators)
ENDIF ( NOT DISABLE_SERVER )
