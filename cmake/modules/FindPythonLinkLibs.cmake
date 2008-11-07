#
# FindPythonLinkLibs.cmake
#
# Find Python libraries to link against
#
# See http://www.itk.org/Bug/view.php?id=2257 for a complete thread on
# the problem
#

# python -c 'import distutils.sysconfig as s; c = s.get_config_var; print c("LOCALMODLIBS"), c("LIBS")'
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "from sys import stdout; import distutils.sysconfig as s; c = s.get_config_var; stdout.write(c(\"LOCALMODLIBS\")); stdout.write(c(\"LIBS\"))" OUTPUT_VARIABLE PYTHON_LINK_LIBS)
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; sys.stdout.write(str(sys.version_info[0]));" OUTPUT_VARIABLE PYTHON_MAJOR_VERSION)
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; sys.stdout.write(str(sys.version_info[1]));" OUTPUT_VARIABLE PYTHON_MINOR_VERSION)

#SET(PYTHON_SHARED_LIBRARY "-lpython${PYTHON_MAJOR_VERSION}.${PYTHON_MINOR_VERSION}")
#SET(PYTHON_LINK_LIBS "${PYTHON_SHARED_LIBRARY} ${PYTHON_LINK_LIBS}")
#MESSAGE(STATUS "Python link libs: ${PYTHON_LINK_LIBS}")
