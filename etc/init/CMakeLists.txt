#
# CMakeLists.txt for openwsman/etc/init
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsmand.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/openwsmand.sh)

#
#EXTRA_DIST = openwsmand.sh.in
#
