#
# CMakeLists.txt for openwsman/tests
#

add_subdirectory(client)
add_subdirectory(epr)
add_subdirectory(filter)
add_subdirectory(xml)

IF( BUILD_CUNIT_TESTS )
add_subdirectory(serialization)
ENDIF( BUILD_CUNIT_TESTS )

