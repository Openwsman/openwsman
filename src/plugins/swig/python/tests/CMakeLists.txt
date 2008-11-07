#
# CMakeLists.txt for plugins/swig/python/tests
#

ENABLE_TESTING()

ADD_TEST(plugin_python_loading     python ${CMAKE_CURRENT_SOURCE_DIR}/loading.py)
