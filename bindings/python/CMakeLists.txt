#
# CMakeLists.txt for openwsman/bindings/python
#
#
# !!Attn!!: This creates two files
#  1. _pywsman.so
#  2. pywsman.py
# and the .py file gets imported. See bindings/python/tests.
#


EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print sys.path[2]" OUTPUT_VARIABLE PYTHON_LIB_DIR)
STRING(REPLACE "\n" "" PYTHON_LIB_DIR "${PYTHON_LIB_DIR}")

MESSAGE(STATUS "Python executable: ${PYTHON_EXECUTABLE}")
MESSAGE(STATUS "Python lib dir: ${PYTHON_LIB_DIR}")
MESSAGE(STATUS "Python site dir: ${PYTHON_LIB_DIR}/site-packages")

SET( SWIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/openwsman_wrap.c" )
SET( SWIG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/../openwsman.i" )

ADD_CUSTOM_COMMAND (
   OUTPUT  ${SWIG_OUTPUT}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating wrapper code for Python ..."
   COMMAND ${SWIG_EXECUTABLE} -python -features autodoc -I${CMAKE_SOURCE_DIR}/include -o ${SWIG_OUTPUT} ${SWIG_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../*.i
)

ADD_CUSTOM_TARGET( glue
   DEPENDS ${SWIG_OUTPUT}
)

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall" )

SET(pywsman_SRCS ${SWIG_OUTPUT} )
ADD_LIBRARY( pywsman SHARED ${pywsman_SRCS} )
SET_TARGET_PROPERTIES( pywsman PROPERTIES PREFIX "" )
ADD_DEPENDENCIES( pywsman glue )

INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_PATH} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include )

TARGET_LINK_LIBRARIES( pywsman wsman )
TARGET_LINK_LIBRARIES( pywsman wsman_client )

INSTALL(TARGETS pywsman LIBRARY DESTINATION ${PYTHON_LIB_DIR}/site-packages)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/pywsman.py DESTINATION ${PYTHON_LIB_DIR}/site-packages )
