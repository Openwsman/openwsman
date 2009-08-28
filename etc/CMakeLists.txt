#
# CMakeLists.txt for openwsman/etc
#

add_subdirectory(init)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/owsmangencert.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/owsmangencert.sh)

########### install files ###############

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/owsmangencert.sh DESTINATION ${SYSCONF_DIR}/openwsman )
INSTALL(FILES ssleay.cnf openwsman.conf DESTINATION ${SYSCONF_DIR}/openwsman )
INSTALL(FILES openwsman.pam DESTINATION ${SYSCONF_DIR}/pam.d RENAME openwsman)
