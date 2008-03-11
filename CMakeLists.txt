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

# options

OPTION( BUILD_EXAMPLES "Build examples" YES )
OPTION( BUILD_PYTHON "Build Python bindings" YES )
OPTION( BUILD_RUBY "Build Ruby bindings" YES )
OPTION( BUILD_PERL "Build Perl bindings" NO )
OPTION( ENABLE_EVENTING_SUPPORT "WS-Eventing wanted" YES )
OPTION( WSMAN_DEBUG_VERBOSE "Verbose debug logging" YES )
OPTION( PACKAGE_PLUGIN_DIR "Where plugins are installed" "${LIB_INSTALL_DIR}/openwsman/plugins" )
OPTION( USE_PAM "Use PAM authentication" YES )
IF( ${USE_PAM} )
SET( HAVE_PAM 1 )
ENDIF( ${USE_PAM} )

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
MESSAGE( FATAL_ERROR " openwsman needs pthread" )
ENDIF( NOT CMAKE_USE_PTHREADS_INIT )


FIND_LIBRARY( HAVE_LIBCRYPT "crypt" )
FIND_LIBRARY( HAVE_LIBNSL "nsl" )
FIND_LIBRARY( HAVE_LIBSOCKET "socket" )

# Various include files

INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE( "sys/time.h" TIME_WITH_SYS_TIME )

CHECK_INCLUDE_FILE( "crypt.h" HAVE_CRYPT_H )
CHECK_INCLUDE_FILE( "ctype.h" HAVE_CTYPE_H )
CHECK_INCLUDE_FILE( "CUnit/Basic.h" HAVE_CUNIT_BASIC_H )
CHECK_INCLUDE_FILE( "dirent.h" HAVE_DIRENT_H )
CHECK_INCLUDE_FILE( "dlfcn.h" HAVE_DLFCN_H )
CHECK_INCLUDE_FILE( "ifaddrs.h" HAVE_IFADDRS_H )
CHECK_INCLUDE_FILE( "inttypes.h" HAVE_INTTYPES_H )
CHECK_INCLUDE_FILE( "memory.h" HAVE_MEMORY_H )
CHECK_INCLUDE_FILE( "netinet/in.h" HAVE_NETINET_IN_H )
CHECK_INCLUDE_FILE( "net/if_dl.h" HAVE_NET_IF_DL_H )
CHECK_INCLUDE_FILE( "net/if.h" HAVE_NET_IF_H )
CHECK_INCLUDE_FILE( "pam/pam_appl.h" HAVE_PAM_PAM_APPL_H )
CHECK_INCLUDE_FILE( "pam/pam_misc.h" HAVE_PAM_PAM_MISC_H )
CHECK_INCLUDE_FILE( "pthread.h" HAVE_PTHREAD_H )
CHECK_INCLUDE_FILE( "security/pam_appl.h" HAVE_SECURITY_PAM_APPL_H )
CHECK_INCLUDE_FILE( "security/pam_misc.h" HAVE_SECURITY_PAM_MISC_H )
CHECK_INCLUDE_FILE( "stdarg.h" HAVE_STDARG_H )
CHECK_INCLUDE_FILE( "stdint.h" HAVE_STDINT_H )
CHECK_INCLUDE_FILE( "stdlib.h" HAVE_STDLIB_H )
CHECK_INCLUDE_FILE( "strings.h" HAVE_STRINGS_H )
CHECK_INCLUDE_FILE( "string.h" HAVE_STRING_H )
CHECK_INCLUDE_FILE( "sys/ioctl.h" HAVE_SYS_IOCTL_H )
CHECK_INCLUDE_FILE( "sys/resource.h" HAVE_SYS_RESOURCE_H )
CHECK_INCLUDE_FILE( "sys/select.h" HAVE_SYS_SELECT_H )
CHECK_INCLUDE_FILE( "sys/sendfile.h" HAVE_SYS_SENDFILE_H )
CHECK_INCLUDE_FILE( "sys/signal.h" HAVE_SYS_SIGNAL_H )
CHECK_INCLUDE_FILE( "sys/socket.h" HAVE_SYS_SOCKET_H )
CHECK_INCLUDE_FILE( "sys/sockio.h" HAVE_SYS_SOCKIO_H )
CHECK_INCLUDE_FILE( "sys/stat.h" HAVE_SYS_STAT_H )
CHECK_INCLUDE_FILE( "sys/types.h" HAVE_SYS_TYPES_H )
CHECK_INCLUDE_FILE( "unistd.h" HAVE_UNISTD_H )
CHECK_INCLUDE_FILE( "vararg.h" HAVE_VARARG_H )

# library functions

INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS("getifaddrs" HAVE_GETIFADDRS)
#SIOCGIFHWADDR
#SIOCGARP

SET( DEFAULT_CONFIG_FILE 1 )

CHECK_FUNCTION_EXISTS( "alloca" HAVE_ALLOCA )
IF (NOT HAVE_ALLOCA)
  CHECK_INCLUDE_FILE( "alloca.h" HAVE_ALLOCA_H )
ENDIF (NOT HAVE_ALLOCA)
CHECK_FUNCTION_EXISTS( "bcopy" HAVE_BCOPY )
CHECK_FUNCTION_EXISTS( "crypt" HAVE_CRYPT )
CHECK_FUNCTION_EXISTS( "daemon" HAVE_DAEMON )
CHECK_FUNCTION_EXISTS( "fnmatch" HAVE_FNMATCH )
CHECK_FUNCTION_EXISTS( "getaddrinfo" HAVE_GETADDRINFO )
CHECK_FUNCTION_EXISTS( "getnameinfo" HAVE_GETNAMEINFO )
CHECK_FUNCTION_EXISTS( "getpid" HAVE_GETPID )
CHECK_FUNCTION_EXISTS( "gettimeofday" HAVE_GETTIMEOFDAY )
CHECK_FUNCTION_EXISTS( "gmtime_r" HAVE_GMTIME_R )
CHECK_FUNCTION_EXISTS( "inet_aton" HAVE_INET_ATON )
CHECK_FUNCTION_EXISTS( "inet_ntop" HAVE_INET_NTOP )
CHECK_FUNCTION_EXISTS( "inet_pton" HAVE_INET_PTON )
CHECK_FUNCTION_EXISTS( "sleep" HAVE_SLEEP )
CHECK_FUNCTION_EXISTS( "srandom" HAVE_SRANDOM )
CHECK_FUNCTION_EXISTS( "strsep" HAVE_STRSEP )
CHECK_FUNCTION_EXISTS( "strtok_r" HAVE_STRTOK_R )
CHECK_FUNCTION_EXISTS( "syslog" HAVE_SYSLOG )
CHECK_FUNCTION_EXISTS( "timegm" HAVE_TIMEGM )
CHECK_FUNCTION_EXISTS( "memmove" HAVE_MEMMOVE )
CHECK_FUNCTION_EXISTS( "unlink" HAVE_UNLINK )
CHECK_FUNCTION_EXISTS( "va_copy" HAVE_VA_COPY )


# types

INCLUDE(CheckTypeSize)
SET( CMAKE_REQUIRED_IUNCLUDE /usr/include/netinet )
CHECK_TYPE_SIZE( "in_addr_t" HAVE_IN_ADDR_T )
CHECK_TYPE_SIZE( "in_port_t" HAVE_IN_PORT_T )
CHECK_TYPE_SIZE( "sa_family_t" HAVE_SA_FAMILY_T )
CHECK_TYPE_SIZE("int" SIZEOF_INT )
CHECK_TYPE_SIZE("long" SIZEOF_LONG )
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG )
CHECK_TYPE_SIZE("short" SIZEOF_SHORT )

#
#/* Defined if you have SSL support */
# HAVE_SSL
#
#/* Define if struct sockaddr contains sa_len */
# HAVE_SA_LEN
#
#
#/* Define to 1 if you have the ANSI C header files. */
# STDC_HEADERS
#
#/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
# TIME_WITH_SYS_TIME
#
#
#/* Define to `int' if <sys/types.h> does not define. */
# ssize_t




####################################################################
# CONFIGURATION                                                    #
####################################################################

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

INCLUDE( ${CMAKE_SOURCE_DIR}/VERSION.cmake )

SET(CMAKE_VERBOSE_MAKEFILE ON)

ADD_DEFINITIONS( -DHAVE_CONFIG_H )
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
