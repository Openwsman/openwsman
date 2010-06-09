#
# CMakeLists.txt for openwsman/bindings/perl/test
#
ENABLE_TESTING()

	# loading wsman
ADD_TEST(bindings_perl_loading    ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} loading.pl )

	# predefined stuff
# ADD_TEST(bindings_perl_constants  ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} constants.pl )

        # Xml bindings
# ADD_TEST(bindings_perl_xmldoc     ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} xmldoc.pl )
# ADD_TEST(bindings_perl_xmlnode    ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} xmlnode.pl )
# ADD_TEST(bindings_perl_xmlattr    ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} xmlattr.pl )
	
	# Client
# ADD_TEST(bindings_perl_client     ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} client.pl )
# ADD_TEST(bindings_perl_client_bad ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} client_bad.pl )

	# Transport
# ADD_TEST(bindings_perl_transport  ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} transport.pl )
	
	# ClientOptions
# ADD_TEST(bindings_perl_options    ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} clientoptions.pl )
# ADD_TEST(bindings_perl_debug      ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} debug.pl )
        # Identify
ADD_TEST(bindings_perl_identify   ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} identify.pl )

	# data types
# ADD_TEST(bindings_perl_fault      ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} fault.pl )

        # needs a CIMOM
ADD_TEST(bindings_perl_computer_system ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} CIM_ComputerSystem_EnumInstances.pl )


# ADD_TEST(bindings_perl_operating_system     ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} cim_operating_system.pl )
# ADD_TEST(bindings_perl_cim_process          ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} cim_process.pl )
# ADD_TEST(bindings_perl_system_configuration ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} cim_system_configuration.pl )
# ADD_TEST(bindings_perl_create               ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} create.pl )
# ADD_TEST(bindings_perl_each_child           ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} each_child.pl )
ADD_TEST(bindings_perl_enumerate ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} CIM_EnumClasses.pl )
# ADD_TEST(bindings_perl_get                  ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} get.pl )
# ADD_TEST(bindings_perl_identify             ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} identify.pl )
# ADD_TEST(bindings_perl_invoke               ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} invoke.pl )
# ADD_TEST(bindings_perl_notepad              ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} notepad.pl )
# ADD_TEST(bindings_perl_nsconsts             ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} nsconsts.pl )
# ADD_TEST(bindings_perl_processes            ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} processes.pl )
# ADD_TEST(bindings_perl_pull                 ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} pull.pl )
# ADD_TEST(bindings_perl_put                  ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} put.pl )
# ADD_TEST(bindings_perl_release              ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} release.pl )

       # Needs specific provider
#Linux_EthernetPort_EnumInstances.pl
#Linux_EthernetPort_GetClass.pl
#Linux_EthernetPort_GetInstance.pl
#Linux_NextHopIPRoute_CreateInstance.pl
#Linux_NextHopIPRoute_DeleteInstance.pl
#ADD_TEST(bindings_perl_computersystem_assoc ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} Linux_ComputerSystem_assoc.pl )

	# needs WinRM system
# ADD_TEST(bindings_perl_win32_operatingsystem ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} win32_operatingsystem.pl )
# ADD_TEST(bindings_perl_win32_process         ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} win32_process.pl )
# ADD_TEST(bindings_perl_win32_services        ${CMAKE_CURRENT_SOURCE_DIR}/run ${CMAKE_CURRENT_SOURCE_DIR} win32_services.pl )
