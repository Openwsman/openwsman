#
# (hand-crafted) Makefile for openwsman/bindings/ruby/tests
#

all:
	# loading rwsman
	ruby loading.rb

	# predefined stuff
	ruby constants.rb

        # Xml bindings
	ruby xmldoc.rb
	ruby xmlnode.rb
	ruby xmlattr.rb
	
	# Client
	ruby client.rb
	ruby client_bad.rb
	
	# ClientOptions
	ruby clientoptions.rb
	ruby debug.rb
	
	# Soap
	ruby status.rb

        # Identify
	ruby identify.rb

	ruby cim_computer_system.rb
	ruby cim_operating_system.rb
	ruby cim_process.rb
	ruby cim_system_configuration.rb
	ruby create.rb
	ruby each_child.rb
	ruby enumerate.rb
	ruby get.rb
	ruby identify.rb
	ruby invoke.rb
	ruby loading.rb
	ruby notepad.rb
	ruby nsconsts.rb
	ruby processes.rb
	ruby pull.rb
	ruby put.rb
	ruby release.rb
	ruby transport.rb

	# needs WinRM system
	ruby win32_operatingsystem.rb
	ruby win32_process.rb
	ruby win32_services.rb

	# needs Windows XP
	ruby xp.rb

clean:
	rm -f core*

check: all
