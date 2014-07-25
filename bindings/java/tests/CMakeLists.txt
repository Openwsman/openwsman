#
# CMakeLists.txt for openwsman/bindings/java/test
#
ENABLE_TESTING()

ADD_TEST(bindings_java_loading_compile ${JAVA_COMPILE} -d ${CMAKE_CURRENT_BINARY_DIR} -cp ${CMAKE_CURRENT_BINARY_DIR}/../${jar_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/loading_sharedlib.java )
	# loading jwsman
ADD_TEST(bindings_java_loading     ${JAVA_RUNTIME} -cp ${CMAKE_CURRENT_BINARY_DIR}/../${jar_NAME}:${CMAKE_CURRENT_BINARY_DIR} loading_sharedlib )

	# predefined stuff
# ADD_TEST(bindings_java_constants  ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} constants )

        # Xml bindings
# ADD_TEST(bindings_java_xmldoc     ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} xmldoc )
# ADD_TEST(bindings_java_xmlnode    ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} xmlnode )
# ADD_TEST(bindings_java_xmlattr    ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} xmlattr )
	
	# Client
# ADD_TEST(bindings_java_client     ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} client )
# ADD_TEST(bindings_java_client_bad ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} client_bad )

	# Transport
# ADD_TEST(bindings_java_transport  ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} transport )
	
	# ClientOptions
# ADD_TEST(bindings_java_options    ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} clientoptions )
# ADD_TEST(bindings_java_debug      ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} debug )

        # Identify
ADD_TEST(bindings_java_identify_compile ${JAVA_COMPILE} -d ${CMAKE_CURRENT_BINARY_DIR} -cp ${CMAKE_CURRENT_BINARY_DIR}/../${jar_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/identify.java )
#ADD_TEST(bindings_java_identify     ${JAVA_RUNTIME} -cp ${CMAKE_CURRENT_BINARY_DIR}/../${jar_NAME}:${CMAKE_CURRENT_BINARY_DIR} identify )
ADD_TEST(bindings_java_identify     ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_BINARY_DIR} ${jar_NAME} identify )

        # Invoke
ADD_TEST(bindings_java_invoke_compile ${JAVA_COMPILE} -d ${CMAKE_CURRENT_BINARY_DIR} -cp ${CMAKE_CURRENT_BINARY_DIR}/../${jar_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/hyperv_invoke.java )
ADD_TEST(bindings_java_invoke     ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_BINARY_DIR} ${jar_NAME} hyperv_invoke )

	# data types
# ADD_TEST(bindings_java_fault      ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} fault )

        # needs a CIMOM
# ADD_TEST(bindings_java_computer_system      ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} cim_computer_system )
# ADD_TEST(bindings_java_operating_system     ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} cim_operating_system )
# ADD_TEST(bindings_java_cim_process          ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} cim_process )
# ADD_TEST(bindings_java_system_configuration ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} cim_system_configuration )
# ADD_TEST(bindings_java_create               ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} create )
# ADD_TEST(bindings_java_each_child           ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} each_child )
# ADD_TEST(bindings_java_enumerate            ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} enumerate )
# ADD_TEST(bindings_java_get                  ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} get )
# ADD_TEST(bindings_java_identify             ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} identify )
# ADD_TEST(bindings_java_invoke               ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} invoke )
# ADD_TEST(bindings_java_notepad              ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} notepad )
# ADD_TEST(bindings_java_nsconsts             ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} nsconsts )
# ADD_TEST(bindings_java_processes            ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} processes )
# ADD_TEST(bindings_java_pull                 ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} pull )
# ADD_TEST(bindings_java_put                  ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} put )
# ADD_TEST(bindings_java_release              ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} release )

	# needs WinRM system
# ADD_TEST(bindings_java_win32_operatingsystem ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} win32_operatingsystem )
# ADD_TEST(bindings_java_win32_process         ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} win32_process )
# ADD_TEST(bindings_java_win32_services        ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} win32_services )
