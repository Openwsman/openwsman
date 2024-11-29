#
# CMakeLists.txt for openwsman/
#
# 2.6 minimum because of CMP0005 (escaping defines)
# 2.8.12 minimum because CMake 3.19.7 says so
# 3.27 for new find python commands
cmake_minimum_required(VERSION 3.27)


PROJECT(openwsman)

include(CTest)
enable_testing()
#
# cmake2.6: backward compatibility to cmake 2.4
#
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0005 NEW)
  if ( POLICY CMP0042 )
    cmake_policy(SET CMP0042 NEW)
  endif ( POLICY CMP0042 )
  if ( POLICY CMP0046 )
    cmake_policy(SET CMP0046 NEW)
  endif ( POLICY CMP0046 )
  # new find python commands
  cmake_policy(SET CMP0148 NEW)
endif(COMMAND cmake_policy)
	      
# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
# SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

INCLUDE( ${CMAKE_SOURCE_DIR}/VERSION.cmake )
SET(VERSION "${OPENWSMAN_MAJOR}.${OPENWSMAN_MINOR}.${OPENWSMAN_PATCH}")
   MESSAGE(STATUS "Version ${VERSION}")
SET(PACKAGE_VERSION \"${OPENWSMAN_MAJOR}.${OPENWSMAN_MINOR}\")
   MESSAGE(STATUS "Package Version ${PACKAGE_VERSION}")
SET(OPENWSMAN_PLUGIN_API_VERSION \"${OPENWSMAN_PLUGIN_API_MAJOR}.${OPENWSMAN_PLUGIN_API_MINOR}\")
   MESSAGE(STATUS "Plugin API Version ${OPENWSMAN_PLUGIN_API_VERSION}")

# Package architecture
IF ( NOT DEFINED PACKAGE_ARCHITECTURE )
 EXECUTE_PROCESS(COMMAND "uname" "-m" OUTPUT_VARIABLE UNAME_M)
 # strip trailing newline
 STRING(REPLACE "\n" "" PACKAGE_ARCHITECTURE ${UNAME_M})
ENDIF ( NOT DEFINED PACKAGE_ARCHITECTURE )
MESSAGE(STATUS "Building for ${PACKAGE_ARCHITECTURE}" )

# pkg-config
find_package(PkgConfig)

# Library path (lib / lib64 )

IF ( DEFINED LIB )
  SET ( LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${LIB}" )
ELSE ( DEFINED  LIB )
  IF (APPLE)
    SET(LIB_SUFFIX "")
  ELSE (APPLE)
    IF (CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET( LIB_SUFFIX "64" )
    ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
  ENDIF (APPLE)
  SET ( LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}" )
ENDIF ( DEFINED  LIB )
MESSAGE(STATUS "Libraries will be installed in ${LIB_INSTALL_DIR}" )
SET( BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin" )
SET( INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include" )
IF( "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
  SET( SYSCONFDIR "/etc" )
ELSE( "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
  SET( SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc" )
ENDIF( "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr")
SET( WSMANCONF_DIR "${SYSCONFDIR}/openwsman" )
MESSAGE(STATUS "Configuration will be installed in ${WSMANCONF_DIR}" )
SET( DEFAULT_CONFIG_FILE \"${WSMANCONF_DIR}/openwsman.conf\")
SET( DEFAULT_CLIENT_CONFIG_FILE \"${WSMANCONF_DIR}/openwsman_client.conf\")
SET( PACKAGE_SUBSCRIPTION_DIR /var/lib/openwsman/subscriptions )
SET( PACKAGE_PLUGIN_DIR ${LIB_INSTALL_DIR}/openwsman/plugins )
SET( PACKAGE_AUTH_DIR ${LIB_INSTALL_DIR}/openwsman/authenticators )


# Macro for error message when header files not present, but build option
# explicitly declared
MACRO(NO_HEADERS_WARNING_EXPL optionName pkgName)
   MESSAGE(STATUS "************  WARNING  *************")
   MESSAGE(STATUS "Didn't find ${pkgName} headers... ")
   MESSAGE(STATUS "${optionName} explicitly requested.")
   MESSAGE(STATUS "Make sure ${pkgName} header files are available and run cmake again.")
   MESSAGE(STATUS "************************************")
   SET(HAVE_WARNINGS YES)
ENDMACRO(NO_HEADERS_WARNING_EXPL)
    
# Macro for error message when header files not present, but build option
# not explicitly declared
MACRO(NO_HEADERS_WARNING optionName pkgName)
   SET(${optionName} NO)
   SET(HAVE_WARNINGS YES)
   MESSAGE(STATUS "************  WARNING  *************")
   MESSAGE(STATUS "Didn't find ${pkgName} headers... not building ${pkgName} bindings")
   MESSAGE(STATUS "************************************")
ENDMACRO(NO_HEADERS_WARNING)

# For binding options, see if it is explicitly set on the command line,
# before defining the option with a default.  Then if we don't find
# the headers: if it was explicitly set then fail, otherwise
# just give a warning message
SET( BUILD_PYTHON_EXPLICIT ${BUILD_PYTHON})
SET( BUILD_PYTHON3_EXPLICIT ${BUILD_PYTHON3})
SET( BUILD_RUBY_EXPLICIT ${BUILD_RUBY})
SET( BUILD_PERL_EXPLICIT ${BUILD_PERL})
SET( BUILD_JAVA_EXPLICIT ${BUILD_JAVA})
SET( BUILD_SWIG_PLUGIN_EXPLICIT ${BUILD_SWIG_PLUGIN})

# options

OPTION( BUILD_LIBCIM "Build CIM plugin" YES )
OPTION( BUILD_EXAMPLES "Build examples" YES )
OPTION( BUILD_BINDINGS "Build client bindings and swig server plugin" YES )
OPTION( BUILD_PYTHON3 "Build Python3 bindings" YES )
OPTION( BUILD_PYTHON "Build Python2 bindings" NO )
IF (BUILD_PYTHON3)
IF (BUILD_PYTHON)
MESSAGE(FATAL_ERROR "Can only build for Python2 or Python3, but not both")
ENDIF (BUILD_PYTHON)
ENDIF (BUILD_PYTHON3)
OPTION( BUILD_RUBY "Build Ruby bindings" YES )
IF( BUILD_RUBY )
OPTION( BUILD_RUBY_GEM "Build Ruby GEM" YES )
ENDIF( BUILD_RUBY )
OPTION( BUILD_PERL "Build Perl bindings" YES )
OPTION( BUILD_JAVA "Build Java bindings" YES )
OPTION( BUILD_CSHARP "Build C# bindings" YES)
OPTION( BUILD_CUNIT_TESTS "Build serialization tests" NO )
OPTION( DISABLE_PLUGINS "Do not build plugins" NO )
OPTION( BUILD_SWIG_PLUGIN "Build SWIG plugin" YES )
OPTION( DISABLE_SERVER "Do not build server component" NO )
OPTION( ENABLE_EVENTING_SUPPORT "WS-Eventing wanted" YES )
OPTION( WSMAN_DEBUG_VERBOSE "Verbose debug logging" NO )
OPTION( ENABLE_IPV6 "Enable IPv6 support" YES )
OPTION( BUILD_TESTS "Build tests" YES )
IF(APPLE)
  OPTION( BUILD_SHARED_LIBS "Build shared libraries" NO )
ELSE(APPLE)
  OPTION( BUILD_SHARED_LIBS "Build shared libraries" YES )
ENDIF(APPLE)

IF (UNIX)
add_definitions(-fPIC)
ENDIF (UNIX)

IF (ENABLE_EVENTING_SUPPORT)
  SET(ENABLE_EVENTING_SUPPORT 1)
  find_package(OpenSSL REQUIRED)
ELSE (ENABLE_EVENTING_SUPPORT)
  SET(ENABLE_EVENTING_SUPPORT 0)
ENDIF (ENABLE_EVENTING_SUPPORT)

IF(WSMAN_DEBUG_VERBOSE)
  SET(WSMAN_DEBUG_VERBOSE 1)
ELSE(WSMAN_DEBUG_VERBOSE)
  SET(WSMAN_DEBUG_VERBOSE 0)
ENDIF(WSMAN_DEBUG_VERBOSE)

OPTION( USE_PAM "Use PAM authentication" YES )
IF( USE_PAM )
  FIND_LIBRARY( HAVE_LIBPAM "pam" )
  IF(HAVE_LIBPAM)
    SET( HAVE_PAM 1 )
    MESSAGE(STATUS "building PAM authentication" )
  ELSE(HAVE_LIBPAM)
    MESSAGE(WARNING " libpam not found, disabling PAM authentication" )
    SET( HAVE_PAM 1 )
  ENDIF(HAVE_LIBPAM)
ELSE( USE_PAM )
 SET( HAVE_PAM 0 )
 MESSAGE(STATUS "Disabling PAM authentication" )
ENDIF( USE_PAM )

find_package(OpenSSL)
IF(OPENSSL_FOUND)
  SET(HAVE_SSL 1)
  SET(USE_OPENSSL 1)
ENDIF(OPENSSL_FOUND)

IF( BUILD_RUBY )
  find_package(Ruby)
  IF(RUBY_FOUND)
    MESSAGE(STATUS "Ruby: ${RUBY_EXECUTABLE}" )
    MESSAGE(STATUS "Ruby Library: ${RUBY_LIBRARY}" )
    MESSAGE(STATUS "Ruby Include: ${RUBY_INCLUDE_PATH}" )
    MESSAGE(STATUS "Ruby Site Arch: ${RUBY_SITEARCH_DIR}" )

    MESSAGE(STATUS "Building Ruby bindings" )
  ELSE(RUBY_FOUND)
    MESSAGE(WARNING "Ruby not found" )
    SET(BUILD_RUBY NO)
  ENDIF(RUBY_FOUND)
  IF(NOT RUBY_INCLUDE_PATH)
    IF(BUILD_RUBY_EXPLICIT)
      NO_HEADERS_WARNING_EXPL(BUILD_RUBY Ruby)
    ELSE(BUILD_RUBY_EXPLICIT)
      NO_HEADERS_WARNING(BUILD_RUBY Ruby)
    ENDIF(BUILD_RUBY_EXPLICIT)
  ENDIF(NOT RUBY_INCLUDE_PATH )
ENDIF( BUILD_RUBY )

IF( BUILD_PERL )
  find_package(PerlLibs)
  IF(PERL_EXECUTABLE)
    MESSAGE(STATUS "Building Perl bindings" )
  ELSE(PERL_EXECUTABLE)
    MESSAGE(WARNING "Perl not found" )
    SET(BUILD_PERL NO)
  ENDIF(PERL_EXECUTABLE)
ENDIF( BUILD_PERL )

IF( BUILD_JAVA )
  find_package(Java)
  IF(JAVA_COMPILE)
    find_package(JNI)
    IF(JNI_INCLUDE_DIRS)
      MESSAGE(STATUS "Building Java bindings" )
    ELSE(JNI_INCLUDE_DIRS)
      MESSAGE(WARNING " Java jni.h not found" )
      SET( BUILD_JAVA "NO" )
    ENDIF(JNI_INCLUDE_DIRS)
  ELSE(JAVA_COMPILE)
    MESSAGE(ERROR " Java compiler not found" )
    SET( BUILD_JAVA "NO" )
  ENDIF(JAVA_COMPILE)
ENDIF( BUILD_JAVA )

# swig
IF( BUILD_BINDINGS )
  FIND_PACKAGE(SWIG)
  IF( NOT SWIG_EXECUTABLE)
    FIND_PROGRAM(SWIG_EXECUTABLE
      NAMES swig
      PATHS ${SWIG_DIR} ${SWIG_DIR}/.. ${SWIG_DIR}/../../bin /usr/bin /usr/local/bin ${CMAKE_INSTALL_PREFIX}/bin
    )
    SET(SWIG_VERSION "(unknown version)")
  ELSE ( NOT SWIG_EXECUTABLE)
    # FIND_PACKAGE(SWIG) succeeded, now check the version
    # swig 2.0.4 and lower is known to be broken
    IF (SWIG_VERSION)
      IF(${SWIG_VERSION} VERSION_LESS "2.0.5")
        MESSAGE( FATAL_ERROR "Found SWIG version ${SWIG_VERSION}, please upgrade SWIG to 2.0.5 or later" )
      ENDIF(${SWIG_VERSION} VERSION_LESS "2.0.5")
    ENDIF (SWIG_VERSION)
  ENDIF( NOT SWIG_EXECUTABLE)
ENDIF( BUILD_BINDINGS )

IF ( NOT SWIG_EXECUTABLE )
  MESSAGE( STATUS "SWIG not found, skipping bindings and swig server plugin." )
  SET(BUILD_BINDINGS FALSE)
  IF (BUILD_SWIG_PLUGIN_EXPLICIT)
    MESSAGE( FATAL "Cannot build SWIG server plugin" )
  ENDIF (BUILD_SWIG_PLUGIN_EXPLICIT)
  SET( BUILD_SWIG_PLUGIN NO )
  IF( BUILD_PYTHON3_EXPLICIT OR BUILD_PYTHON_EXPLICIT OR BUILD_RUBY_EXPLICIT OR BUILD_PERL_EXPLICIT OR BUILD_JAVA_EXPLICIT)
    MESSAGE( ERROR "Cannot build client bindings" )
  ENDIF( BUILD_PYTHON3_EXPLICIT OR BUILD_PYTHON_EXPLICIT OR BUILD_RUBY_EXPLICIT OR BUILD_PERL_EXPLICIT OR BUILD_JAVA_EXPLICIT)
ELSE ( NOT SWIG_EXECUTABLE )
  MESSAGE( STATUS "SWIG ${SWIG_VERSION} found at ${SWIG_EXECUTABLE}" )
ENDIF ( NOT SWIG_EXECUTABLE )

# curl

IF(UNIX)
  find_package(CURL)
  IF ( NOT CURL_FOUND)
   MESSAGE( FATAL_ERROR " curl not found" )
  ELSE ( NOT CURL_FOUND)
    INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
    IF(CURL_VERSION_STRING)
      STRING(COMPARE LESS ${CURL_VERSION_STRING} "7.12.0" result)
      IF(result LESS 0)
        MESSAGE( FATAL_ERROR " curl version ${CURL_VERSION_STRING} is too low, need 7.12.0 or greater" )
      ENDIF(result LESS 0)
    ENDIF(CURL_VERSION_STRING)
  ENDIF( NOT CURL_FOUND)
ENDIF(UNIX)

# xml2

find_package(LibXml2)
IF ( NOT LIBXML2_FOUND)
  MESSAGE( FATAL_ERROR " libxml2 not found" )
ELSE ( NOT LIBXML2_FOUND)
  INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
ENDIF( NOT LIBXML2_FOUND)

# pthreads

IF(UNIX)
  find_package(Threads)
  IF( NOT CMAKE_USE_PTHREADS_INIT )
    MESSAGE( FATAL_ERROR " openwsman needs pthread" )
  ELSE( NOT CMAKE_USE_PTHREADS_INIT )
   SET(USE_PTHREAD TRUE)
  ENDIF( NOT CMAKE_USE_PTHREADS_INIT )
ENDIF(UNIX)

# dl

FIND_LIBRARY( HAVE_LIBDL "dl" )
IF(HAVE_LIBDL)
 SET(DL_LIBRARIES ${HAVE_LIBDL})
 SET(HAVE_LIBDL 1)
ELSE(HAVE_LIBDL)
 SET(HAVE_LIBDL 0)
ENDIF(HAVE_LIBDL)

# m

FIND_LIBRARY( HAVE_LIBM "m" )
IF(HAVE_LIBM)
 SET(M_LIBRARIES ${HAVE_LIBM})
 SET(HAVE_LIBM 1)
ELSE(HAVE_LIBM)
 SET(HAVE_LIBM 0)
ENDIF(HAVE_LIBM)

# crypt

FIND_LIBRARY( HAVE_LIBCRYPT "crypt" )
IF(HAVE_LIBCRYPT)
 SET(CRYPT_LIBRARIES ${HAVE_LIBCRYPT})
 SET(HAVE_LIBCRYPT 1)
ELSE(HAVE_LIBCRYPT)
 SET(HAVE_LIBCRYPT 0)
ENDIF(HAVE_LIBCRYPT)

# nsl

FIND_LIBRARY( HAVE_LIBNSL "nsl" )
IF(HAVE_LIBNSL)
 SET(HAVE_LIBNSL 1)
ELSE(HAVE_LIBNSL)
 SET(HAVE_LIBNSL 0)
ENDIF(HAVE_LIBNSL)

# socket

FIND_LIBRARY( HAVE_LIBSOCKET "socket" )
IF(HAVE_LIBSOCKET)
 SET(HAVE_LIBSOCKET 1)
ELSE(HAVE_LIBSOCKET)
 SET(HAVE_LIBSOCKET 0)
ENDIF(HAVE_LIBSOCKET)

# cmpisfcc
IF(BUILD_LIBCIM)
FIND_LIBRARY( SFCC_LIBRARIES "cmpisfcc" REQUIRED )
ENDIF(BUILD_LIBCIM)

# Various include files

INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)

CHECK_FUNCTION_EXISTS( "alloca" HAVE_ALLOCA )
IF (NOT HAVE_ALLOCA)
  SET(HAVE_ALLOCA 0)
  SET(C_ALLOCA 0)
  CHECK_INCLUDE_FILE( "alloca.h" HAVE_ALLOCA_H )
ELSE (NOT HAVE_ALLOCA)
  SET(HAVE_ALLOCA 1)
  SET(C_ALLOCA 1)
  SET(HAVE_ALLOCA_H 0)
ENDIF (NOT HAVE_ALLOCA)

#
# The wsman_config file checks include file existance via "#if",
# requiring a 0/1 argument
# However CHECK_INCLUDE_FILE , returns ""/"1" resulting in
# an empty argument to "#if" in the 'false' case
# The code below ensures that "HAVE_xxx" is set to "0" or "1"
#

SET (FILES_TO_TEST "alloca.h" "crypt.h" "ctype.h" "dirent.h" "dlfcn.h" "ifaddrs.h" "inttypes.h" "memory.h" "netinet/in.h" "net/if_dl.h" "net/if.h" "pam/pam_appl.h" "pam/pam_misc.h" "pthread.h" "security/pam_appl.h" "security/pam_misc.h" "stdarg.h" "stdint.h" "stdlib.h" "strings.h" "string.h" "sys/ioctl.h" "sys/resource.h" "sys/select.h" "sys/sendfile.h" "sys/signal.h" "sys/socket.h" "sys/sockio.h" "sys/stat.h" "sys/types.h" "unistd.h" "vararg.h" )
FOREACH( FILE ${FILES_TO_TEST})
  STRING(REGEX REPLACE "\\." "_" FILEDOT ${FILE})
  STRING(REGEX REPLACE "/" "_" FILESLASH ${FILEDOT})
  STRING(TOUPPER ${FILESLASH} UPNAME)
#  MESSAGE(STATUS "Looking for ${FILE} as ${UPNAME}")
  SET(HAVENAME "HAVE_${UPNAME}")
  CHECK_INCLUDE_FILE( "${FILE}" HAVE_${FILE}_FILE )
  IF(HAVE_${FILE}_FILE)
    SET(${HAVENAME} 1)
  ELSE(HAVE_${FILE}_FILE)
    SET(${HAVENAME} 0)
  ENDIF(HAVE_${FILE}_FILE)
ENDFOREACH( FILE ${FILES_TO_TEST})

IF(BUILD_CUNIT_TESTS)
  CHECK_INCLUDE_FILE( "CUnit/Basic.h" HAVE_CUNIT_BASIC_H )
  IF(HAVE_CUNIT_BASIC_H)
  ELSE(HAVE_CUNIT_BASIC_H)
    MESSAGE(FATAL_ERROR "Can't find CUnit/Basic.h - missing cunit-devel package ?")
  ENDIF(HAVE_CUNIT_BASIC_H)
ENDIF(BUILD_CUNIT_TESTS)

#/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
CHECK_INCLUDE_FILE( "sys/time.h" TIME_WITH_SYS_TIME )
IF(NOT TIME_WITH_SYS_TIME)
 SET(TIME_WITH_SYS_TIME 0)
ENDIF(NOT TIME_WITH_SYS_TIME)

#/* Define ssize_t to int' if <sys/types.h> does not define. */
SET(SSIZE_T_MISSING 0)

# library functions

CHECK_FUNCTION_EXISTS("getifaddrs" HAVE_GETIFADDRS)
#SIOCGIFHWADDR
#SIOCGARP

SET( CRAY_STACKSEG_END 0 )

#
# The wsman_config file checks functions existance via "#if",
# requiring a 0/1 argument
# CHECK_FUNCTION_EXISTS however, returns ""/"1" resulting in
# an empty argument to "#if" in the 'false' case
# The code below ensures that "HAVE_xxx" is set to "0" or "1"
#

SET (FUNCS_TO_TEST "bcopy" "crypt" "daemon" "fnmatch" "getaddrinfo" "getnameinfo" "getpid" "gettimeofday" "gmtime_r" "inet_aton" "inet_ntop" "inet_pton" "sleep" "srandom" "ssl" "strsep" "strtok_r" "syslog" "timegm" "memmove" "unlink" "va_copy" )
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

SET (TYPES_TO_TEST "in_addr_t" "in_port_t" "sa_family_t" "int" "long" "long long" "short")
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


INCLUDE(CheckStructHasMember)
# Check if struct sockaddr contains sa_len
CHECK_STRUCT_HAS_MEMBER("struct sockaddr" sa_len sys/socket.h HAVE_SA_LEN)
IF(HAVE_SA_LEN)
  SET(HAVE_SA_LEN 1)
ELSE(HAVE_SA_LEN)
  SET(HAVE_SA_LEN 0)
ENDIF(HAVE_SA_LEN)
  
IF (ENABLE_IPV6)
  IF(NOT WIN32)
    # Check if struct sockaddr_in6 contains sin6
    CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in6" sin6_addr netinet/in.h HAVE_IPV6)

    IF (HAVE_IPV6)
      SET(ENABLE_IPV6 1)
    ELSE (HAVE_IPV6)
      MESSAGE( SEND_ERROR " IPv6 not supported by system, disabling" )
      SET(ENABLE_IPV6 0)
    ENDIF (HAVE_IPV6)
  ENDIF(NOT WIN32)
ELSE (ENABLE_IPV6)
  SET(ENABLE_IPV6 0)
ENDIF (ENABLE_IPV6)

#
#/* Define to 1 if you have the ANSI C header files. */
# STDC_HEADERS
SET(STDC_HEADERS 1)

####################################################################
# CONFIGURATION                                                    #
####################################################################

ADD_DEFINITIONS( -DHAVE_CONFIG_H )

SET(PACKAGE "openwsman")
SET(PACKAGE_BUGREPORT "\"www.openwsman.org\"")
SET(PACKAGE_NAME "\"${PACKAGE}\"")
SET(PACKAGE_STRING "\"WS-Management for all\"")

SET("prefix" ${CMAKE_INSTALL_PREFIX})
SET("includedir" ${CMAKE_INSTALL_PREFIX}/include)
SET("libdir" ${LIB_INSTALL_DIR})
# Library names !
SET(WSMAN_PKG wsman)
SET(WSMAN_SERVER_PKG wsman_server)
SET(WSMAN_CLIENT_PKG wsman_client)
SET(WSMAN_CLIENTPP_PKG wsman_clientpp)
IF(UNIX)
 SET(WSMAN_CLIENT_TRANSPORT_PKG wsman_curl_client_transport)
ELSE(UNIX)
 SET(WSMAN_CLIENT_TRANSPORT_PKG wsman_win_client_transport)
ENDIF(UNIX)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/wsman_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/wsman_config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman.pc.in ${CMAKE_CURRENT_BINARY_DIR}/openwsman.pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman++.pc.in ${CMAKE_CURRENT_BINARY_DIR}/openwsman++.pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman-server.pc.in ${CMAKE_CURRENT_BINARY_DIR}/openwsman-server.pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/package/openwsman.spec.in ${CMAKE_CURRENT_BINARY_DIR}/package/openwsman.spec @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/package/openwsman.service.in ${CMAKE_CURRENT_BINARY_DIR}/package/openwsman.service)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/openwsman.pc
              ${CMAKE_CURRENT_BINARY_DIR}/openwsman++.pc
	      ${CMAKE_CURRENT_BINARY_DIR}/openwsman-server.pc DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

OPTION(MSVC_STATIC_RUNTIME "Use multi-threaded statically-linked runtime library" NO)
IF(MSVC_STATIC_RUNTIME)
 add_compile_options(/MT$<$<CONFIG:Debug>:d>)
ENDIF(MSVC_STATIC_RUNTIME)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(etc)

IF(BUILD_BINDINGS)
  add_subdirectory(bindings)
ENDIF(BUILD_BINDINGS)

IF ( BUILD_TESTS )
add_subdirectory(tests)
ENDIF ( BUILD_TESTS )

IF ( BUILD_EXAMPLES )
add_subdirectory(examples)
ENDIF ( BUILD_EXAMPLES )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include )

if (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS /sdl /W3")
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /GS /sdl /W3")
else (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Woverloaded-virtual -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wunused -Werror" )
SET( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -g -Wall -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wunused -Werror" )
SET( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3" )
SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3" )
endif (CMAKE_CXX_COMPILER_ID MATCHES MSVC)

####################################################################
# PACKAGING                                                        #
####################################################################
SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Web Services for Management")
SET(CPACK_PACKAGE_VENDOR "openwsman.org")
SET(CPACK_PACKAGE_VERSION_MAJOR ${OPENWSMAN_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${OPENWSMAN_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${OPENWSMAN_PATCH})
SET(CPACK_GENERATOR "TBZ2")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_PACKAGE_DIRECTORY "package")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE}-${VERSION}" )
   MESSAGE(STATUS "Package file ${CPACK_SOURCE_PACKAGE_FILE_NAME}")

# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
SET (CPACK_SOURCE_IGNORE_FILES
#git files
"/.git"
"\\\\.gitignore$"
#svn files
"\\\\.svn/"
"\\\\.cvsignore$"
# temporary files
"\\\\.swp$"
# backup files
"~$"
# eclipse files
"\\\\.cdtproject$"
"\\\\.cproject$"
"\\\\.project$"
"\\\\.settings/"
# others
"\\\\.#"
"/#"
"/build/"
"/_build/"
"/\\\\.git/"
# used before
"/CVS/"
"/\\\\.libs/"
"/\\\\.deps/"
"\\\\.o$"
"\\\\.lo$"
"\\\\.la$"
"Makefile\\\\.in$"
"Makefile$"
# autotool
"/m4/"
"autom4te.cache"
"config.log"
"./config.h"
"configure$"
"config.status"
"depcomp"
"config.guess"
"install.sh"
"libtool"
# generated
"\\\\.bz2$"
"\\\\.class$"
"/bindings/ruby/rdoc/"
)

INCLUDE(CPack)

####################################################################

SET( DOC_INSTALL_DIR
   "${CMAKE_INSTALL_PREFIX}/share/doc/packages/${PACKAGE}"
   CACHE PATH "The install dir for documentation (default prefix/share/doc/packages/${PACKAGE})"
   FORCE
)

IF(HAVE_WARNINGS)
 MESSAGE(STATUS "   ************ NOTE: ************")
 MESSAGE(STATUS "   Warnings occurred during cmake configuration... Please see output")
 MESSAGE(STATUS "   *******************************")
ENDIF(HAVE_WARNINGS) 

####################################################################
# RPM SPEC                                                         #
####################################################################

SET( AUTOBUILD_COMMAND
  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/package/*.tar.bz2
  COMMAND mkdir -p _CPack_Packages/${CPACK_TOPLEVEL_TAG}
  COMMAND ${CMAKE_MAKE_PROGRAM} package_source
  COMMAND ${CMAKE_COMMAND} -E copy ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2 ${CMAKE_BINARY_DIR}/package
  COMMAND ${CMAKE_COMMAND} -E remove ${CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/openwsman-redhat-initscript.patch" "${CMAKE_BINARY_DIR}/package"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/openwsman.pam" "${CMAKE_BINARY_DIR}/package"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/openwsman.pam.rh" "${CMAKE_BINARY_DIR}/package"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/openwsman.rpmlintrc" "${CMAKE_BINARY_DIR}/package"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/openwsman.SuSEfirewall2" "${CMAKE_BINARY_DIR}/package"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/package/openwsman.firewalld" "${CMAKE_BINARY_DIR}/package"
)

ADD_CUSTOM_TARGET( srcpackage_local
  ${AUTOBUILD_COMMAND}
)

ADD_CUSTOM_TARGET( srcpackage
  ${AUTOBUILD_COMMAND}
)
