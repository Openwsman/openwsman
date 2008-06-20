#
# CMakeLists.txt for openwsmann/src/plugins

add_subdirectory(identify)
add_subdirectory(wsman)

if(BUILD_LIBCIM)
add_subdirectory(cim)
endif(BUILD_LIBCIM)

if(BUILD_SWIG_PLUGIN)
add_subdirectory(swig)
endif(BUILD_SWIG_PLUGIN)
