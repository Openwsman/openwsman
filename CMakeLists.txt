#
# CMakeLists.txt for openwsman/
#

PROJECT(openwsman)

# Library path (lib / lib64 )

IF ( DEFINED LIB )
  SET ( LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${LIB}" )
ELSE ( DEFINED  LIB )
  IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
    SET( LIB_SUFFIX "64" )
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
  SET ( LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" )
ENDIF ( DEFINED  LIB )
MESSAGE(STATUS "Libraries will be installed in ${LIB_INSTALL_DIR}" )
SET( BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" )
SET( INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include" )
SET( SYSCONF_DIR "/etc" )
SET( PACKAGE_SUBSCRIPTION_DIR "/var/lib/openwsman/subscriptions" )
SET( PACKAGE_PLUGIN_DIR "${LIB_INSTALL_DIR}/openwsman/plugins" )
SET( PACKAGE_AUTH_DIR "${LIB_INSTALL_DIR}/openwsman/authenticators" )

# options

OPTION( BUILD_LIBCIM "Build CIM plugin" YES )
OPTION( BUILD_EXAMPLES "Build examples" YES )
OPTION( BUILD_PYTHON "Build Python bindings" YES )
OPTION( BUILD_RUBY "Build Ruby bindings" YES )
OPTION( BUILD_PERL "Build Perl bindings" NO )
OPTION( BUILD_CUNIT_TESTS "Build serialization tests" NO )
OPTION( DISABLE_PLUGINS "Do not build plugins" NO )
OPTION( DISABLE_SERVER "Do not build server component" NO )
OPTION( ENABLE_EVENTING_SUPPORT "WS-Eventing wanted" YES )
OPTION( WSMAN_DEBUG_VERBOSE "Verbose debug logging" YES )
IF( ${WSMAN_DEBUG_VERBOSE})
 SET(WSMAN_DEBUG_VERBOSE 1)
ELSE( ${WSMAN_DEBUG_VERBOSE})
 SET(WSMAN_DEBUG_VERBOSE 0)
ENDIF( ${WSMAN_DEBUG_VERBOSE})

OPTION( PACKAGE_PLUGIN_DIR "Where plugins are installed" "${LIB_INSTALL_DIR}/openwsman/plugins" )
OPTION( USE_PAM "Use PAM authentication" YES )
IF( ${USE_PAM} )
 SET( HAVE_PAM 1 )
ELSE( ${USE_PAM} )
 SET( HAVE_PAM 0 )
ENDIF( ${USE_PAM} )

INCLUDE(FindOpenSSL)
IF(OPENSSL_FOUND)
SET(HAVE_SSL TRUE)
SET(USE_OPENSSL TRUE)
ENDIF(OPENSSL_FOUND)

IF( BUILD_RUBY )
INCLUDE(FindRuby)
EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print Config::CONFIG['sitearchdir']"
  OUTPUT_VARIABLE RUBY_SITEARCH_DIR)
MESSAGE(STATUS "Building Ruby bindings" )
ENDIF( BUILD_RUBY )

IF( BUILD_PYTHON )
INCLUDE(FindPythonInterp)
INCLUDE(FindPythonLibs)
MESSAGE(STATUS "Building Python bindings" )
ENDIF( BUILD_PYTHON )

IF( BUILD_PERL )
INCLUDE(FindPerl)
INCLUDE(FindPerlLibs)
MESSAGE(STATUS "Building Perl bindings" )
ENDIF( BUILD_PERL )

# curl

INCLUDE(FindCURL)
IF ( NOT CURL_FOUND)
  MESSAGE( FATAL_ERROR " curl not found" )
ELSE ( NOT CURL_FOUND)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
ENDIF( NOT CURL_FOUND)

# xml2

INCLUDE(FindLibXml2)
IF ( NOT LIBXML2_FOUND)
  MESSAGE( FATAL_ERROR " libxml2 not found" )
ELSE ( NOT LIBXML2_FOUND)
  INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
ENDIF( NOT LIBXML2_FOUND)

# pthreads

INCLUDE(FindThreads)
IF( NOT CMAKE_USE_PTHREADS_INIT )
 maMESSAGE( FATAL_ERROR " openwsman needs pthread" )
ELSE( NOT CMAKE_USE_PTHREADS_INIT )
 SET(USE_PTHREAD TRUE)
ENDIF( NOT CMAKE_USE_PTHREADS_INIT )


FIND_LIBRARY( HAVE_LIBCRYPT "crypt" )
IF(HAVE_LIBCRYPT)
 SET(HAVE_LIBCRYPT 1)
ELSE(HAVE_LIBCRYPT)
 SET(HAVE_LIBCRYPT 0)
ENDIF(HAVE_LIBCRYPT)

FIND_LIBRARY( HAVE_LIBNSL "nsl" )
IF(HAVE_LIBNSL)
 SET(HAVE_LIBNSL 1)
ELSE(HAVE_LIBNSL)
 SET(HAVE_LIBNSL 0)
ENDIF(HAVE_LIBNSL)

FIND_LIBRARY( HAVE_LIBSOCKET "socket" )
IF(HAVE_LIBSOCKET)
 SET(HAVE_LIBSOCKET 1)
ELSE(HAVE_LIBSOCKET)
 SET(HAVE_LIBSOCKET 0)
ENDIF(HAVE_LIBSOCKET)

# Various include files

INCLUDE(CheckIncludeFile)

#
# The wsman_config file checks include file existance via "#if",
# requiring a 0/1 argument
# HoweverHECK_INCLUDE_FILE , returns ""/"1" resulting in
# an empty argument to "#if" in the 'false' case
# The code below ensures that "HAVE_xxx" is set to "0" or "1"
#

SET (FILES_TO_TEST "crypt.h" "ctype.h" "CUnit/Basic.h" "dirent.h" "dlfcn.h" "ifaddrs.h" "inttypes.h" "memory.h" "netinet/in.h" "net/if_dl.h" "net/if.h" "pam/pam_appl.h" "pam/pam_misc.h" "pthread.h" "security/pam_appl.h" "security/pam_misc.h" "stdarg.h" "stdint.h" "stdlib.h" "strings.h" "string.h" "sys/ioctl.h" "sys/resource.h" "sys/select.h" "sys/sendfile.h" "sys/signal.h" "sys/socket.h" "sys/sockio.h" "sys/stat.h" "sys/types.h" "unistd.h" "vararg.h" )
#SET(FILES_TO_TEST "crypt.h")
FOREACH( FILE ${FILES_TO_TEST})
  STRING(REGEX REPLACE "\\." "_" FILEDOT ${FILE})
  STRING(REGEX REPLACE "/" "_" FILESLASH ${FILEDOT})
  STRING(TOUPPER ${FILESLASH} UPNAME)
#  MESSAGE(STATUS "Looking for ${FILE} as ${UPNAME}")
  SET(HAVENAME "HAVE_${UPNAME}")
  CHECK_INCLUDE_FILE( "${FILE}" HAVE_FILE )
  IF(HAVE_FILE)
    SET(${HAVENAME} 1)
  ELSE(HAVE_FILE)
    SET(${HAVENAME} 0)
  ENDIF(HAVE_FILE)
ENDFOREACH( FILE ${FILES_TO_TEST})

#/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
CHECK_INCLUDE_FILE( "sys/time.h" TIME_WITH_SYS_TIME )
IF(NOT TIME_WITH_SYS_TIME)
 SET(TIME_WITH_SYS_TIME 0)
ENDIF(NOT TIME_WITH_SYS_TIME)

#/* Define ssize_t to int' if <sys/types.h> does not define. */
SET(SSIZE_T_MISSING 0)

# library functions

INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS("getifaddrs" HAVE_GETIFADDRS)
#SIOCGIFHWADDR
#SIOCGARP

SET( DEFAULT_CONFIG_FILE 1 )
SET( CRAY_STACKSEG_END 0 )

CHECK_FUNCTION_EXISTS( "alloca" HAVE_ALLOCA )
IF (NOT HAVE_ALLOCA)
  SET(HAVE_ALLOCA 0)
  SET(C_ALLOCA 0)
  CHECK_INCLUDE_FILE( "alloca.h" HAVE_ALLOCA_H )
ENDIF (NOT HAVE_ALLOCA)

#
# The wsman_config file checks functions existance via "#if",
# requiring a 0/1 argument
# CHECK_FUNCTION_EXISTS however, returns ""/"1" resulting in
# an empty argument to "#if" in the 'false' case
# The code below ensures that "HAVE_xxx" is set to "0" or "1"
#

SET (FUNCS_TO_TEST "bcopy" "crypt" "daemon" "fnmatch" "getaddrinfo" "getnameinfo" "getpid" "gettimeofday" "gmtime_r" "inet_aton" "inet_ntop" "inet_pton" "sleep" "srandom" "strsep" "strtok_r" "syslog" "timegm" "memmove" "unlink" "va_copy" )
FOREACH( FUNC ${FUNCS_TO_TEST})
  STRING(TOUPPER ${FUNC} UPNAME)
  SET(HAVENAME "HAVE_${UPNAME}")
  CHECK_FUNCTION_EXISTS( "${FUNC}" HAVE_FUNC )
  IF(HAVE_FUNC)
    SET(${HAVENAME} 1)
  ELSE(HAVE_FUNC)
    SET(${HAVENAME} 0)
  ENDIF(HAVE_FUNC)
ENDFOREACH( FUNC ${FUNCS_TO_TEST})


# types

INCLUDE(CheckTypeSize)
SET( CMAKE_REQUIRED_IUNCLUDE "/usr/include/netinet" )

#
# The wsman_config file checks type existance via "#if",
# requiring a 0/1 argument
# CHECK_TYPE_SIZE however, returns ""/"1" resulting in
# an empty argument to "#if" in the 'false' case
# The code below ensures that "HAVE_xxx" is set to "0" or "1"
#

SET (TYPES_TO_TEST "in_addr_t" "in_port_t" "sa_family_t" "int" "long" "long long" "short" )
FOREACH( TYPE ${TYPES_TO_TEST})
  STRING(TOUPPER ${TYPE} UPNAME)
  STRING(REGEX REPLACE " " "_" UPNAME ${UPNAME})
  SET(HAVENAME "HAVE_${UPNAME}")
  CHECK_TYPE_SIZE( "${TYPE}" HAVE_TYPE )
  MESSAGE(STATUS "Check type ${TYPE} as ${HAVENAME}")
  IF(HAVE_TYPE)
    SET(${HAVENAME} 1)
  ELSE(HAVE_TYPE)
    SET(${HAVENAME} 0)
  ENDIF(HAVE_TYPE)
ENDFOREACH( TYPE ${TYPES_TO_TEST})


#
#/* Define if struct sockaddr contains sa_len */
# HAVE_SA_LEN
SET(HAVE_SA_LEN 0)
#
#
#/* Define to 1 if you have the ANSI C header files. */
# STDC_HEADERS
SET(STDC_HEADERS 1)

####################################################################
# CONFIGURATION                                                    #
####################################################################

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

INCLUDE( ${CMAKE_SOURCE_DIR}/VERSION.cmake )
SET(VERSION "\"${OPENWSMAN_MAJOR}.${OPENWSMAN_MINOR}.${OPENWSMAN_PATCH}\"")

SET(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

SET(PACKAGE "\"openwsman\"")
SET(PACKAGE_BUGREPORT "\"www.openwsman.org\"")
SET(PACKAGE_NAME "\"openwsman\"")
SET(PACKAGE_STRING "\"WS-Management for all\"")
SET(PACKAGE_TARNAME "\"openwsman-${VERSION}.tar.bz2\"")
SET(PACKAGE_VERSION "${VERSION}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wsman_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/wsman_config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/openwsman.pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman++.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/openwsman++.pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman-server.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/openwsman-server.pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman.spec.cmake ${CMAKE_CURRENT_BINARY_DIR}/openwsman.spec)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(etc)
add_subdirectory(tests)

FIND_PROGRAM(SWIG_EXECUTABLE
  NAMES swig-1.3 swig
    PATHS ${SWIG_DIR} ${SWIG_DIR}/.. ${SWIG_DIR}/../../bin /usr/bin /usr/local/bin ${CMAKE_INSTALL_PREFIX}/bin)
IF ( NOT SWIG_EXECUTABLE )
  MESSAGE( FATAL "SWIG not found." )
ELSE ( NOT SWIG_EXECUTABLE )
  MESSAGE( STATUS "SWIG found at ${SWIG_EXECUTABLE}" )
  ADD_SUBDIRECTORY(bindings)
ENDIF ( NOT SWIG_EXECUTABLE )

IF ( BUILD_EXAMPLES )
add_subdirectory(examples)
ENDIF ( BUILD_EXAMPLES )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Woverloaded-virtual" )
SET( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -g -Wall" )
SET( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3" )
SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3" )

########### install files ###############

install(FILES  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/openwsman )

#AUTOMAKE_OPTIONS = no-dist-gzip dist-bzip2
#
#package: dist
#	cp openwsman.spec /usr/src/packages/SPEC
#	cp openwsman-*.tar.gz /usr/src/packages/SOURCES
#	rpmbuild -ba openwsman.spec
#
#etags: TAGS
#	find . -name "*.[chCH]" -print | etags -
#
#install-data-local: openwsman.pc
#	$(mkinstalldirs) $(DESTDIR)$(libdir)/pkgconfig
#	$(INSTALL_DATA)  openwsman.pc $(DESTDIR)$(libdir)/pkgconfig
#	$(INSTALL_DATA)  openwsman++.pc $(DESTDIR)$(libdir)/pkgconfig
#	$(INSTALL_DATA)  openwsman-server.pc $(DESTDIR)$(libdir)/pkgconfig
#
#uninstall-local:
#	rm $(DESTDIR)$(libdir)/pkgconfig/openwsman.pc
#	rm $(DESTDIR)$(libdir)/pkgconfig/openwsman++.pc
#	rm $(DESTDIR)$(libdir)/pkgconfig/openwsman-server.pc
#
#EXTRA_DIST = OpenWsmanLib.vcproj OpenWsmanLib.sln
