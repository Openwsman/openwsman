#
# CMakeLists.txt for openwsman/bindings/python/test
#
ENABLE_TESTING()

	# loading wsman
ADD_TEST(bindings_python_loading      ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} loading.py )

	# predefined stuff
# ADD_TEST(bindings_python_constants  ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} constants.py )

        # Xml bindings
# ADD_TEST(bindings_python_xmldoc     ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} xmldoc.py )
# ADD_TEST(bindings_python_xmlnode    ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} xmlnode.py )
# ADD_TEST(bindings_python_xmlattr    ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} xmlattr.py )
	
	# Client
# ADD_TEST(bindings_python_client     ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} client.py )
# ADD_TEST(bindings_python_client_bad ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} client_bad.py )

	# Transport
# ADD_TEST(bindings_python_transport  ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} transport.py )
	
	# ClientOptions
# ADD_TEST(bindings_python_options    ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} clientoptions.py )
# ADD_TEST(bindings_python_debug      ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} debug.py )
        # Identify
ADD_TEST(bindings_python_identify     ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} identify.py )

	# data types
# ADD_TEST(bindings_python_fault      ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} fault.py )

ADD_TEST(bindings_python_epr          ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} epr.py )

        # needs a CIMOM
# ADD_TEST(bindings_python_computer_system      ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} cim_computer_system.py )
# ADD_TEST(bindings_python_operating_system     ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} cim_operating_system.py )
# ADD_TEST(bindings_python_cim_process          ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} cim_process.py )
# ADD_TEST(bindings_python_system_configuration ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} cim_system_configuration.py )
# ADD_TEST(bindings_python_create               ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} create.py )
# ADD_TEST(bindings_python_each_child           ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} each_child.py )
ADD_TEST(bindings_python_enumerate            ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} enumerate.py )
ADD_TEST(bindings_python_add_selector         ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} add_selector.py )
ADD_TEST(bindings_python_add_selector2        ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} add_selector2.py )
# ADD_TEST(bindings_python_get                  ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} get.py )
# ADD_TEST(bindings_python_identify             ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} identify.py )
# ADD_TEST(bindings_python_invoke               ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} invoke.py )
# ADD_TEST(bindings_python_notepad              ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} notepad.py )
# ADD_TEST(bindings_python_nsconsts             ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} nsconsts.py )
# ADD_TEST(bindings_python_processes            ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} processes.py )
# ADD_TEST(bindings_python_pull                 ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} pull.py )
# ADD_TEST(bindings_python_put                  ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} put.py )
ADD_TEST(bindings_python_release              ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} release.py )

	# needs WinRM system
# ADD_TEST(bindings_python_win32_operatingsystem ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} win32_operatingsystem.py )
# ADD_TEST(bindings_python_win32_process         ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} win32_process.py )
# ADD_TEST(bindings_python_win32_services        ${CMAKE_CURRENT_SOURCE_DIR}/run ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR} win32_services.py )
