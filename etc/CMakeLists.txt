#
# CMakeLists.txt for openwsman/etc
#

add_subdirectory(init)
add_subdirectory(pam)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/owsmangencert.sh.cmake ${CMAKE_CURRENT_BINARY_DIR}/owsmangencert.sh)

########### install files ###############

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/owsmangencert.sh DESTINATION ${WSMANCONF_DIR} )
INSTALL(FILES ssleay.cnf openwsman.conf openwsman_client.conf DESTINATION ${WSMANCONF_DIR} )
