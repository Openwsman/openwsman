#
# CMakeLists.txt for openwsman/include/u
#

SET( OWSMAN_INCLUDES buf.h carpal.h libu.h log.h logprv.h memory.h misc.h os.h uri.h uuid.h lock.h strings.h md5.h list.h hash.h base64.h iniparser.h debug.h debug_internal.h uerr.h uoption.h gettimeofday.h syslog.h pthreadx.h )

install(FILES ${OWSMAN_INCLUDES} DESTINATION ${INCLUDE_DIR}/openwsman/u)

