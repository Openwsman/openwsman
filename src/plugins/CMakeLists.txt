#
# CMakeLists.txt for openwsmann/src/plugins

enable_testing()

add_subdirectory(identify)
add_subdirectory(wsman)
add_subdirectory(redirect)

if(BUILD_LIBCIM)
add_subdirectory(cim)
endif(BUILD_LIBCIM)

if(BUILD_SWIG_PLUGIN)
add_subdirectory(swig)
endif(BUILD_SWIG_PLUGIN)
