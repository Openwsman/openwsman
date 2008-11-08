#
# CMakeLists.txt for openwsman/bindings/ruby/test
#
ENABLE_TESTING()

	# loading rwsman
ADD_TEST(bindings_ruby_loading    ruby ${CMAKE_CURRENT_SOURCE_DIR}/loading.rb )

	# predefined stuff
ADD_TEST(bindings_ruby_constants  ruby ${CMAKE_CURRENT_SOURCE_DIR}/constants.rb )

        # Xml bindings
ADD_TEST(bindings_ruby_xmldoc     ruby ${CMAKE_CURRENT_SOURCE_DIR}/xmldoc.rb )
ADD_TEST(bindings_ruby_xmlnode    ruby ${CMAKE_CURRENT_SOURCE_DIR}/xmlnode.rb )
ADD_TEST(bindings_ruby_xmlattr    ruby ${CMAKE_CURRENT_SOURCE_DIR}/xmlattr.rb )
	
	# Client
ADD_TEST(bindings_ruby_client     ruby ${CMAKE_CURRENT_SOURCE_DIR}/client.rb )
ADD_TEST(bindings_ruby_client_bad ruby ${CMAKE_CURRENT_SOURCE_DIR}/client_bad.rb )

	# Transport
ADD_TEST(bindings_ruby_transport  ruby ${CMAKE_CURRENT_SOURCE_DIR}/transport.rb )
	
	# ClientOptions
ADD_TEST(bindings_ruby_options    ruby ${CMAKE_CURRENT_SOURCE_DIR}/clientoptions.rb )
ADD_TEST(bindings_ruby_debug      ruby ${CMAKE_CURRENT_SOURCE_DIR}/debug.rb )
        # Identify
ADD_TEST(bindings_ruby_identify   ruby ${CMAKE_CURRENT_SOURCE_DIR}/identify.rb )

        # needs a CIMOM
ADD_TEST(bindings_ruby_computer_system      ruby ${CMAKE_CURRENT_SOURCE_DIR}/cim_computer_system.rb )
ADD_TEST(bindings_ruby_operating_system     ruby ${CMAKE_CURRENT_SOURCE_DIR}/cim_operating_system.rb )
ADD_TEST(bindings_ruby_cim_process          ruby ${CMAKE_CURRENT_SOURCE_DIR}/cim_process.rb )
ADD_TEST(bindings_ruby_system_configuration ruby ${CMAKE_CURRENT_SOURCE_DIR}/cim_system_configuration.rb )
ADD_TEST(bindings_ruby_create               ruby ${CMAKE_CURRENT_SOURCE_DIR}/create.rb )
ADD_TEST(bindings_ruby_each_child           ruby ${CMAKE_CURRENT_SOURCE_DIR}/each_child.rb )
ADD_TEST(bindings_ruby_enumerate            ruby ${CMAKE_CURRENT_SOURCE_DIR}/enumerate.rb )
ADD_TEST(bindings_ruby_get                  ruby ${CMAKE_CURRENT_SOURCE_DIR}/get.rb )
ADD_TEST(bindings_ruby_identify             ruby ${CMAKE_CURRENT_SOURCE_DIR}/identify.rb )
ADD_TEST(bindings_ruby_invoke               ruby ${CMAKE_CURRENT_SOURCE_DIR}/invoke.rb )
ADD_TEST(bindings_ruby_notepad              ruby ${CMAKE_CURRENT_SOURCE_DIR}/notepad.rb )
ADD_TEST(bindings_ruby_nsconsts             ruby ${CMAKE_CURRENT_SOURCE_DIR}/nsconsts.rb )
ADD_TEST(bindings_ruby_processes            ruby ${CMAKE_CURRENT_SOURCE_DIR}/processes.rb )
ADD_TEST(bindings_ruby_pull                 ruby ${CMAKE_CURRENT_SOURCE_DIR}/pull.rb )
ADD_TEST(bindings_ruby_put                  ruby ${CMAKE_CURRENT_SOURCE_DIR}/put.rb )
ADD_TEST(bindings_ruby_release              ruby ${CMAKE_CURRENT_SOURCE_DIR}/release.rb )

	# needs WinRM system
ADD_TEST(bindings_ruby_win32_operatingsystem ruby ${CMAKE_CURRENT_SOURCE_DIR}/win32_operatingsystem.rb )
ADD_TEST(bindings_ruby_win32_process         ruby ${CMAKE_CURRENT_SOURCE_DIR}/win32_process.rb )
ADD_TEST(bindings_ruby_wind32_services       ruby ${CMAKE_CURRENT_SOURCE_DIR}/win32_services.rb )

	# needs Windows XP
ADD_TEST(bindings_ruby_xp ruby ${CMAKE_CURRENT_SOURCE_DIR}/xp.rb )
