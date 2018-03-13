#
# CMakeLists.txt for plugins/swig/python
#

enable_testing()
add_subdirectory(tests)

SET (BUILD_SHARED_LIBS ON)

FIND_PACKAGE(PythonInterp REQUIRED)

EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "from sys import stdout; from distutils import sysconfig; stdout.write(sysconfig.get_python_lib())" OUTPUT_VARIABLE PYTHON_LIB_DIR)

IF (NOT PYTHON_SITE_DIR)
  SET (PYTHON_SITE_DIR ${PYTHON_LIB_DIR})
ENDIF (NOT PYTHON_SITE_DIR)

MESSAGE(STATUS "Python executable: ${PYTHON_EXECUTABLE}")
MESSAGE(STATUS "Python inc dir: ${PYTHON_INCLUDE_DIRS}")
MESSAGE(STATUS "Python lib dir: ${PYTHON_LIB_DIR}")
#MESSAGE(STATUS "Python libraries: ${PYTHON_LIBRARIES}")
MESSAGE(STATUS "Python site dir: ${PYTHON_SITE_DIR}")

SET( SWIG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/swig_wrap.c" )
SET( SWIG_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/../plugin.i" )

ADD_CUSTOM_COMMAND (
   OUTPUT  ${SWIG_OUTPUT}
   COMMAND ${CMAKE_COMMAND} -E echo_append "Creating wrapper code for Python ..."
   COMMAND ${SWIG_EXECUTABLE} -python -threads -features autodoc -I${CMAKE_SOURCE_DIR}/include -I${CMAKE_SOURCE_DIR}/bindings -o ${SWIG_OUTPUT} ${SWIG_INPUT}
   COMMAND ${CMAKE_COMMAND} -E echo "Done."
   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../*.i
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../src/*.c
   DEPENDS ${CMAKE_SOURCE_DIR}/bindings/*.i
)

SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g" )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../src )
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/bindings )
# wsman_config.h is generated
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )
# all other wsman includes
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/include )
INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_DIRS} )

#
# wsman_python_plugin.so
#

SET( NAME wsman_python_plugin )
ADD_LIBRARY( ${NAME} ${SWIG_OUTPUT})
#TARGET_LINK_LIBRARIES( ${NAME} ${PYTHON_LIBRARIES} )
TARGET_LINK_LIBRARIES( ${NAME} python${PYTHON_MAJOR_VERSION}.${PYTHON_MINOR_VERSION} )
TARGET_LINK_LIBRARIES( ${NAME} pthread )

INSTALL(TARGETS ${NAME} LIBRARY DESTINATION ${PACKAGE_PLUGIN_DIR})

# .py: swig generated
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/wsman_plugin.py DESTINATION ${PYTHON_SITE_DIR} )


#
# pywsmanplugin.py: plugin implementation
#
INSTALL(FILES pywsmanplugin.py DESTINATION ${PYTHON_SITE_DIR} )

# compile to .pyc
INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c \"from py_compile import compile; compile('\$ENV{DESTDIR}${PYTHON_SITE_DIR}/wsman_plugin.py', dfile='${PYTHON_SITE_DIR}/wsman_plugin.py')\")")
INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c \"from py_compile import compile; compile('\$ENV{DESTDIR}${PYTHON_SITE_DIR}/pywsmanplugin.py', dfile='${PYTHON_SITE_DIR}/pywsmanplugin.py')\")")
