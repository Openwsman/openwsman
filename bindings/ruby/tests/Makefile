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
	
	# ClientOptions
	ruby clientoptions.rb
	
clean:
	rm -f core*

check: all
