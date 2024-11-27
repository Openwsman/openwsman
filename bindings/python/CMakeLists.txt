#
# CMakeLists.txt for openwsman/bindings/python
#
#
# !!Attn!!: This creates two files
#  1. _pywsman.so
#  2. pywsman.py
# and the .py file gets imported. See bindings/python/tests.
#

enable_testing()

add_subdirectory(tests)

IF( BUILD_PYTHON3 )
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
ELSEIF( BUILD_PYTHON2 )
find_package(Python2 COMPONENTS Interpreter Development REQUIRED)
ENDIF( BUILD_PYTHON3 )

IF (Python2_FOUND)
SET(PYTHON_EXECUTABLE ${Python2_EXECUTABLE})
SET(PYTHON_LIB_DIR ${Python2_LIBRARY_DIRS})
SET(PYTHON_INCLUDE_DIRS ${Python2_INCLUDE_DIRS})
SET(PYTHON_INCLUDE_PATH ${Python2_INCLUDE_PATH})
SET(PYTHON_STDLIB ${Python2_STDLIB})
SET(PYTHON_STDARCH ${Python2_STDARCH})
SET(PYTHON_SITELIB ${Python2_SITELIB})
SET(PYTHON_SITEARCH ${Python2_SITEARCH})
ELSEIF (Python3_FOUND)
SET(PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
SET(PYTHON_LIB_DIR ${Python3_LIBRARY_DIRS})
SET(PYTHON_INCLUDE_DIRS ${Python3_INCLUDE_DIRS})
SET(PYTHON_INCLUDE_PATH ${Python3_INCLUDE_PATH})
SET(PYTHON_STDLIB ${Python3_STDLIB})
SET(PYTHON_STDARCH ${Python3_STDARCH})
SET(PYTHON_SITELIB ${Python3_SITELIB})
SET(PYTHON_SITEARCH ${Python3_SITEARCH})
ELSE (Python3_FOUND)
  MESSAGE(STATUS "No Python found")
ENDIF (Python2_FOUND)

# CMake 3.30 and Python 3.11 do not set _STDLIB, _STDARCH, _SITELIB, or _SITEARCH

# CMake + Python not always define proper STD or SITE directories :-/
execute_process(
  COMMAND "${PYTHON_EXECUTABLE}" -c "if True:
    import sysconfig as sc
    print(sc.get_path(name='stdlib'))"
  OUTPUT_VARIABLE PYTHON_SITE
  OUTPUT_STRIP_TRAILING_WHITESPACE)            

SET(PYTHON_INSTALL_DIR "${PYTHON_SITE}/site-packages")

MESSAGE(STATUS "Python executable: ${PYTHON_EXECUTABLE}")
MESSAGE(STATUS "Python lib dir: ${PYTHON_LIB_DIR}")
MESSAGE(STATUS "Python std lib dir: ${Python_STDLIB}")
MESSAGE(STATUS "Python std arch dir: ${Python_STDARCH}")
MESSAGE(STATUS "Python site lib dir: ${Python_SITELIB}")
MESSAGE(STATUS "Python site arch dir: ${Python_SITEARCH}")
MESSAGE(STATUS "Python site dir: ${PYTHON_SITE}")
MESSAGE(STATUS "Python install dir: ${PYTHON_INSTALL_DIR}")

SET( SWIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/openwsman_wrap.c" )

ADD_CUSTOM_COMMAND (
   OUTPUT  ${SWIG_OUTPUT}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating wrapper code for Python ..."
   COMMAND ${SWIG_EXECUTABLE} -python -features autodoc -features autodoc -I${CMAKE_BINARY_DIR}/bindings -I${CMAKE_SOURCE_DIR}/bindings -I${CMAKE_SOURCE_DIR}/include -o ${SWIG_OUTPUT} ${SWIG_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../*.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )

SET(pywsman_SRCS ${SWIG_OUTPUT} ${CMAKE_CURRENT_SOURCE_DIR}/../openwsman.c)
ADD_LIBRARY( pywsman ${pywsman_SRCS} )
SET_TARGET_PROPERTIES( pywsman PROPERTIES PREFIX "_" )

INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_DIRS} )
# RHEL5 needs this:
INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_PATH} )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/bindings ${CMAKE_SOURCE_DIR}/include )

TARGET_LINK_LIBRARIES( pywsman wsman )
TARGET_LINK_LIBRARIES( pywsman wsman_client )

INSTALL(TARGETS pywsman LIBRARY DESTINATION ${PYTHON_INSTALL_DIR})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/pywsman.py DESTINATION ${PYTHON_INSTALL_DIR} )
