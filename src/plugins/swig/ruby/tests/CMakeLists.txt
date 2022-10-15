#
# CMakeLists.txt for openwsman/src/plugins/swig/ruby/test
#
ENABLE_TESTING()
ADD_TEST(plugin_ruby_loading ruby -C ${CMAKE_CURRENT_SOURCE_DIR} plugin_loading.rb )
