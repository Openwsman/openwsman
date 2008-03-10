include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR} )


########### install files ###############

install(FILES  $(OWSMAN_INCLUDES) DESTINATION /include/openwsman/u)



#original Makefile.am contents follow:

#
#wsmanincludedir = $(includedir)/openwsman/u
#wsmaninclude_HEADERS = $(OWSMAN_INCLUDES)
#OWSMAN_INCLUDES = buf.h carpal.h   \
#				libu.h log.h logprv.h memory.h \
#				misc.h os.h uri.h \
#				uuid.h lock.h strings.h md5.h list.h \
#				hash.h base64.h iniparser.h  \
#				debug.h uerr.h uoption.h gettimeofday.h syslog.h \
#				pthreadx.h
# 
