#
# CMakeLists.txt for openwsman/etc/init
#

# Prevent CMake confusion with shell lines like "${..."
SET(X1 "$")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsmand.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/openwsmand.sh)

#
#EXTRA_DIST = openwsmand.sh.in
#
