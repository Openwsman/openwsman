#
# CMakeLists.txt for openwsman/bindings/ruby/test
#
ENABLE_TESTING()

	# loading rwsman
ADD_TEST(bindings_ruby_loading    ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} loading.rb )

	# predefined stuff
ADD_TEST(bindings_ruby_constants  ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} constants.rb )

	# version
ADD_TEST(bindings_ruby_version    ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} version.rb )

	# EPR uri
ADD_TEST(bindings_ruby_uri_prefix ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} uri_prefix.rb )

        # Xml bindings
ADD_TEST(bindings_ruby_xmldoc     ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} xmldoc.rb )
ADD_TEST(bindings_ruby_xmlnode    ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} xmlnode.rb )
ADD_TEST(bindings_ruby_xmlattr    ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} xmlattr.rb )
ADD_TEST(bindings_ruby_xmlfind    ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} xmlfind.rb )
	
	# Client
ADD_TEST(bindings_ruby_client     ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} client.rb )
ADD_TEST(bindings_ruby_client_bad ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} client_bad.rb )

	# Transport
ADD_TEST(bindings_ruby_transport  ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} transport.rb )
	
	# ClientOptions
ADD_TEST(bindings_ruby_options    ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} clientoptions.rb )
ADD_TEST(bindings_ruby_debug      ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} debug.rb )
        # Identify
ADD_TEST(bindings_ruby_identify   ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} identify.rb )

	# data types
ADD_TEST(bindings_ruby_fault      ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} fault.rb )

        # needs a CIMOM
ADD_TEST(bindings_ruby_computer_system      ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} cim_computer_system.rb )
ADD_TEST(bindings_ruby_operating_system     ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} cim_operating_system.rb )
ADD_TEST(bindings_ruby_cim_process          ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} cim_process.rb )
ADD_TEST(bindings_ruby_system_configuration ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} cim_system_configuration.rb )
ADD_TEST(bindings_ruby_create               ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} create.rb )
ADD_TEST(bindings_ruby_each_child           ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} each_child.rb )
ADD_TEST(bindings_ruby_enumerate            ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} enumerate.rb )
ADD_TEST(bindings_ruby_get                  ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} get.rb )
ADD_TEST(bindings_ruby_identify             ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} identify.rb )
ADD_TEST(bindings_ruby_invoke               ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} invoke.rb )
ADD_TEST(bindings_ruby_notepad              ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} notepad.rb )
ADD_TEST(bindings_ruby_nsconsts             ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} nsconsts.rb )
#ADD_TEST(bindings_ruby_pull                 ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} pull.rb )
#ADD_TEST(bindings_ruby_put                  ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} put.rb )
#ADD_TEST(bindings_ruby_release              ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} release.rb )

	# needs WinRM system
#ADD_TEST(bindings_ruby_win32_operatingsystem ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} win32_operatingsystem.rb )
#ADD_TEST(bindings_ruby_win32_process         ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} win32_process.rb )
#ADD_TEST(bindings_ruby_win32_services        ${CMAKE_CURRENT_SOURCE_DIR}/ruby -I. -C ${CMAKE_CURRENT_SOURCE_DIR} win32_services.rb )
