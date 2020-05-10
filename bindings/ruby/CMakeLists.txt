#
# CMakeLists.txt for openwsman/bindings/ruby
#

enable_testing()

add_subdirectory(tests)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print (RbConfig::CONFIG['ruby_version']||RbConfig::CONFIG['RUBY_PROGRAM_VERSION'])" OUTPUT_VARIABLE RUBY_VERSION)

# prefer vendor dirs
SET(RUBY_WSMAN_LIB_DIR ${RUBY_VENDORLIB_DIR})
SET(RUBY_WSMAN_ARCH_DIR ${RUBY_VENDORARCH_DIR})

MESSAGE(STATUS "Ruby executable: ${RUBY_EXECUTABLE}")
MESSAGE(STATUS "Ruby version: ${RUBY_VERSION}")
MESSAGE(STATUS "Ruby dir: ${RUBY_WSMAN_LIB_DIR}")
MESSAGE(STATUS "Ruby arch dir: ${RUBY_WSMAN_ARCH_DIR}")
MESSAGE(STATUS "Ruby include path: ${RUBY_INCLUDE_PATH}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman/version.rb.in ${CMAKE_CURRENT_BINARY_DIR}/openwsman/version.rb)

SET( SWIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/openwsman_wrap.c" )

ADD_CUSTOM_COMMAND (
   OUTPUT  ${SWIG_OUTPUT}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating wrapper code for Ruby..."
   COMMAND ${SWIG_EXECUTABLE} -ruby -autorename -DRUBY_VERSION=${RUBY_VERSION_MAJOR}${RUBY_VERSION_MINOR} -I${CMAKE_BINARY_DIR}/bindings -I${CMAKE_SOURCE_DIR}/bindings -I${CMAKE_SOURCE_DIR}/include -o ${SWIG_OUTPUT} ${SWIG_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../*.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )

SET(rbwsman_SRCS ${SWIG_OUTPUT} ${CMAKE_CURRENT_SOURCE_DIR}/../openwsman.c )
ADD_LIBRARY( rbwsman ${rbwsman_SRCS} )
# prevent 'lib...'
SET_TARGET_PROPERTIES( rbwsman PROPERTIES PREFIX "" )
SET_TARGET_PROPERTIES( rbwsman PROPERTIES OUTPUT_NAME "_openwsman" )

INCLUDE_DIRECTORIES( ${RUBY_INCLUDE_PATH} )
INCLUDE_DIRECTORIES( ${RUBY_INCLUDE_PATH}/${RUBY_ARCH} )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/bindings ${CMAKE_SOURCE_DIR}/include )

TARGET_LINK_LIBRARIES( rbwsman wsman )
TARGET_LINK_LIBRARIES( rbwsman wsman_client )
TARGET_LINK_LIBRARIES( rbwsman ${LIBXML2_LIBRARIES} )

# copy .so file to ARCH_DIR
INSTALL(TARGETS rbwsman LIBRARY DESTINATION ${RUBY_WSMAN_ARCH_DIR})

# copy openwsman/*.rb to LIB_DIR/openwsman/*.rb
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/openwsman DESTINATION ${RUBY_WSMAN_LIB_DIR}
        PATTERN ".rb"
	PATTERN ".svn" EXCLUDE
	PATTERN "*.in" EXCLUDE
	PATTERN "*~" EXCLUDE
	PATTERN "Makefile.am" EXCLUDE)

# copy generated openwsman/version.rb
INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/openwsman DESTINATION ${RUBY_WSMAN_LIB_DIR}
        PATTERN ".rb")

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/openwsman.rb DESTINATION ${RUBY_WSMAN_LIB_DIR})

# rdoc
SET(rdoc_dir "${CMAKE_CURRENT_BINARY_DIR}/html")
ADD_CUSTOM_COMMAND (
   OUTPUT ${rdoc_dir}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating rdoc documentation ..."
   COMMAND rm -rf ${rdoc_dir}
   COMMAND ./rdoc ${RUBY_VERSION_MAJOR}.${RUBY_VERSION_MINOR} -o ${rdoc_dir} -t "Openwsman - WS-Management for all" -m README.rdoc README.rdoc ../openwsman.i ../*.i openwsman/*.rb
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
   DEPENDS ${CMAKE_SOURCE_DIR}/bindings/*.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/openwsman/*.rb
)

ADD_CUSTOM_TARGET(ruby_rdoc ALL DEPENDS "${rdoc_dir}")
ADD_DEPENDENCIES(ruby_rdoc rbwsman)



IF( BUILD_RUBY_GEM )
#
# Ruby GEM
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/openwsman.gemspec.in ${CMAKE_CURRENT_BINARY_DIR}/openwsman.gemspec)

SET(GEM_NAME "openwsman-${VERSION}.gem")
ADD_CUSTOM_TARGET(ruby_gem ALL DEPENDS ${GEM_NAME} ${SWIG_OUTPUT})
#ADD_DEPENDENCIES(ruby_gem )

ADD_CUSTOM_COMMAND (
   OUTPUT  ${GEM_NAME}
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   COMMAND "gem" "build" "${CMAKE_CURRENT_BINARY_DIR}/openwsman.gemspec"
   DEPENDS "ext"
   DEPENDS ${rdoc_dir}
   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/openwsman.gemspec
)

#
# Prep GEM files
# gemspec needs _relative_ paths
# so create ext and lib here and copy files
#
ADD_CUSTOM_COMMAND (
  OUTPUT "ext"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND "rm" "-rf" "lib"
  COMMAND "rm" "-rf" "ext"
  COMMAND "mkdir" "-p" "ext/openwsman"
  COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/../openwsman.c" "ext/openwsman"
  COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/../openwsman.h" "ext/openwsman"
  COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/extconf.rb" "ext/openwsman"
  COMMAND "cp" "${CMAKE_CURRENT_BINARY_DIR}/../version.i" "ext/openwsman"
  COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/../*.i" "ext/openwsman"
  COMMAND "mkdir" "-p" "ext/ruby"
  COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/helpers.h" "ext/ruby"

  COMMAND "mkdir" "-p" "lib/openwsman"
  COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/openwsman.rb" "lib"
  COMMAND "cp" "${CMAKE_CURRENT_SOURCE_DIR}/openwsman/*.rb" "lib/openwsman"
  COMMAND "cp" "${CMAKE_CURRENT_BINARY_DIR}/openwsman/version.rb" "lib/openwsman"

  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/extconf.rb
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/openwsman.rb
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/openwsman/*.rb
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/openwsman/*.rb.in
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../*.i
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../*.i
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/openwsman/version.rb
)

ENDIF( BUILD_RUBY_GEM )

#
# Leave this to %doc in the .spec file
#INSTALL(DIRECTORY "${rdoc_dir}" DESTINATION ${DOC_INSTALL_DIR})

#
#
SET_DIRECTORY_PROPERTIES( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "ext;lib")
