add_subdirectory(identify)
add_subdirectory(wsman)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = identify wsman
#
#if BUILD_LIBCIM
#SUBDIRS += cim
#endif
