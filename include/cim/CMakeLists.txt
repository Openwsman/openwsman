include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )


########### install files ###############

install(FILES  cim-interface.h DESTINATION /include/openwsman/cim)



#original Makefile.am contents follow:

#cimincludedir = $(includedir)/openwsman/cim
#
#ciminclude_HEADERS = cim-interface.h
#cim_INCLUDES  = cim-interface.h
#
