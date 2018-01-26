#
# CMakeLists.txt for openwsman/bindings/perl
#
#
# !!Attn!!: This creates two files
#  1. openwsman.so
#  2. openwsman.pm
# and the .pm file gets loaded. See bindings/perl/tests.
#

enable_testing()

add_subdirectory(tests)

EXECUTE_PROCESS(COMMAND ${PERL_EXECUTABLE} -e "use Config; print \$Config{ccflags}" OUTPUT_VARIABLE PERL_CCFLAGS)
EXECUTE_PROCESS(COMMAND ${PERL_EXECUTABLE} -e "use Config; print \$Config{archlib}.\"/CORE\"" OUTPUT_VARIABLE PERL_CORE_DIR)
EXECUTE_PROCESS(COMMAND ${PERL_EXECUTABLE} -e "use Config; print \$Config{installvendorarch}" OUTPUT_VARIABLE PERL_VENDOR_ARCH)
EXECUTE_PROCESS(COMMAND ${PERL_EXECUTABLE} -e "use Config; print \$Config{installvendorlib}" OUTPUT_VARIABLE PERL_VENDOR_LIB)
EXECUTE_PROCESS(COMMAND ${PERL_EXECUTABLE} -e "use Config; print \$Config{ccdlflags}" OUTPUT_VARIABLE PERL_LINK_FLAGS)


MESSAGE(STATUS "Perl executable: ${PERL_EXECUTABLE}")
MESSAGE(STATUS "Perl core dir: ${PERL_CORE_DIR}")
MESSAGE(STATUS "Perl vendor arch dir: ${PERL_VENDOR_ARCH}")
MESSAGE(STATUS "Perl vendor lib dir: ${PERL_VENDOR_LIB}")

SET( SWIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/openwsman_wrap.c" )

ADD_DEFINITIONS( ${PERL_CCFLAGS} -Wno-unused -Wno-error -Wno-nonnull)
LINK_DIRECTORIES( ${PERL_CORE_DIR} )

ADD_CUSTOM_COMMAND (
   OUTPUT  ${SWIG_OUTPUT}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating wrapper code for Perl ..."
   COMMAND ${SWIG_EXECUTABLE} -perl -features autodoc -I${CMAKE_BINARY_DIR}/bindings -I${CMAKE_SOURCE_DIR}/bindings -I${CMAKE_SOURCE_DIR}/include -o ${SWIG_OUTPUT} ${SWIG_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../*.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )

SET(wsman_SRCS ${SWIG_OUTPUT} ${CMAKE_CURRENT_SOURCE_DIR}/../openwsman.c)
ADD_LIBRARY( plwsman ${wsman_SRCS} )
SET_TARGET_PROPERTIES( plwsman PROPERTIES PREFIX "" )
SET_TARGET_PROPERTIES( plwsman PROPERTIES OUTPUT_NAME "openwsman" )

INCLUDE_DIRECTORIES( ${PERL_CORE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/bindings )

TARGET_LINK_LIBRARIES( plwsman wsman )
TARGET_LINK_LIBRARIES( plwsman wsman_client )

INSTALL(TARGETS plwsman LIBRARY DESTINATION ${PERL_VENDOR_ARCH} )
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/openwsman.pm DESTINATION ${PERL_VENDOR_LIB} )
