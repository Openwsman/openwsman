#
# CMakeLists.txt for openwsmann/src/plugins

#
add_subdirectory(identify)
add_subdirectory(wsman)

if(BUILD_LIBCIM)
add_subdirectory(cim)
endif(BUILD_LIBCIM)
