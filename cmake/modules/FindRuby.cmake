# - Find Ruby
# This module finds if Ruby is installed and determines where the include files
# and libraries are. It also determines what the name of the library is. This
# code sets the following variables:
#
#  RUBY_INCLUDE_PATH = path to where ruby.h can be found
#  RUBY_EXECUTABLE   = full path to the ruby binary

# Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
# See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.


if(RUBY_LIBRARY AND RUBY_INCLUDE_PATH)
   # Already in cache, be silent
   set(RUBY_FIND_QUIETLY TRUE)
endif (RUBY_LIBRARY AND RUBY_INCLUDE_PATH)

#   RUBY_ARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",RbConfig::CONFIG@<:@"archdir"@:>@)'`
#   RUBY_SITEARCHDIR=`$RUBY -r rbconfig -e 'printf("%s",RbConfig::CONFIG@<:@"sitearchdir"@:>@)'`
#   RUBY_SITEDIR=`$RUBY -r rbconfig -e 'printf("%s",RbConfig::CONFIG@<:@"sitelibdir"@:>@)'`
#   RUBY_LIBDIR=`$RUBY -r rbconfig -e 'printf("%s",RbConfig::CONFIG@<:@"libdir"@:>@)'`
#   RUBY_LIBRUBYARG=`$RUBY -r rbconfig -e 'printf("%s",RbConfig::CONFIG@<:@"LIBRUBYARG_SHARED"@:>@)'`

FIND_PROGRAM(RUBY_EXECUTABLE NAMES ruby.ruby2.5 ruby.ruby2.4 ruby.ruby2.3 ruby.ruby2.2 ruby ruby22 ruby21 ruby20 ruby19 ruby1.8 ruby18 )

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['MAJOR']"
   OUTPUT_VARIABLE RUBY_VERSION_MAJOR)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['MINOR']"
   OUTPUT_VARIABLE RUBY_VERSION_MINOR)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['TEENY']"
   OUTPUT_VARIABLE RUBY_VERSION_PATCH)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['archdir']"
   OUTPUT_VARIABLE RUBY_ARCH_DIR)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['rubyhdrdir']"
   OUTPUT_VARIABLE RUBY_HDR_DIR ERROR_QUIET)
   
EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['arch']"
   OUTPUT_VARIABLE RUBY_ARCH)
   
EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['libdir']"
   OUTPUT_VARIABLE RUBY_POSSIBLE_LIB_PATH)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['rubylibdir']"
   OUTPUT_VARIABLE RUBY_RUBY_LIB_PATH)

# site_ruby
EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['sitearchdir']"
   OUTPUT_VARIABLE RUBY_SITEARCH_DIR)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['sitelibdir']"
   OUTPUT_VARIABLE RUBY_SITELIB_DIR)

EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['vendorarchdir']"
  OUTPUT_VARIABLE RUBY_VENDORARCH_DIR ERROR_QUIET)

IF(RUBY_VENDORARCH_DIR)
  IF(${RUBY_VENDORARCH_DIR} STREQUAL "nil")
  ELSE(${RUBY_VENDORARCH_DIR} STREQUAL "nil")
    EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['vendorlibdir']"
       OUTPUT_VARIABLE RUBY_VENDORLIB_DIR)
  ENDIF(${RUBY_VENDORARCH_DIR} STREQUAL "nil")
ENDIF(RUBY_VENDORARCH_DIR)

IF(NOT RUBY_VENDORLIB_DIR)

    # fall back to site*dir
    EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['sitearchdir']"
       OUTPUT_VARIABLE RUBY_VENDORARCH_DIR)

    EXECUTE_PROCESS(COMMAND ${RUBY_EXECUTABLE} -r rbconfig -e "print RbConfig::CONFIG['sitelibdir']"
       OUTPUT_VARIABLE RUBY_VENDORLIB_DIR)

ENDIF(NOT RUBY_VENDORLIB_DIR)

# this is not needed if you use "print" inside the ruby statements
# remove the new lines from the output by replacing them with empty strings
#STRING(REPLACE "\n" "" RUBY_ARCH_DIR "${RUBY_ARCH_DIR}")
#STRING(REPLACE "\n" "" RUBY_POSSIBLE_LIB_PATH "${RUBY_POSSIBLE_LIB_PATH}")
#STRING(REPLACE "\n" "" RUBY_RUBY_LIB_PATH "${RUBY_RUBY_LIB_PATH}")


FIND_PATH(RUBY_INCLUDE_PATH
   NAMES ruby.h
  PATHS
   ${RUBY_HDR_DIR}
   ${RUBY_ARCH_DIR}
   /usr/lib/ruby/1.8/i586-linux-gnu/ )

FIND_LIBRARY(RUBY_LIBRARY
  NAMES ruby2.5 ruby2.4 ruby2.3 ruby2.2 ruby2.1 ruby ruby1.9 ruby1.8
  PATHS
   ${RUBY_POSSIBLE_LIB_PATH}
   ${RUBY_RUBY_LIB_PATH}
  )

MARK_AS_ADVANCED(
  RUBY_EXECUTABLE
  RUBY_LIBRARY
  RUBY_INCLUDE_PATH
  )
