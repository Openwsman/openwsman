# -*- encoding: utf-8 -*-

SRCDIR = File.dirname(__FILE__)
BINDIR = Dir.getwd  # only true for CMake :-/

require File.join(BINDIR, "version.rb")

#
# Prep GEM files
# s.files below needs _relative_ pathes
# so create ext and lib here and copy files
#
Dir.mkdir("ext") unless File.directory?("ext")
Dir.mkdir("ext/openwsman") unless File.directory?("ext/openwsman")
Dir.mkdir("ext/ruby") unless File.directory?("ext/ruby")
system "cp -f #{SRCDIR}/../openwsman.c ext"
system "cp -f *.c ext/openwsman"
system "cp -f #{SRCDIR}/../openwsman.h ext/openwsman"
system "cp -f #{SRCDIR}/helpers.h ext/ruby"
system "cp -f #{SRCDIR}/extconf.rb ext/openwsman"

Dir.mkdir("lib") unless File.directory?("lib")
system "cp -f #{SRCDIR}/openwsman.rb lib"
system "cp -a -f #{SRCDIR}/openwsman lib"

Gem::Specification.new do |s|
  s.name        = "openwsman"
  s.version     = OPENWSMAN_VERSION
  s.platform    = Gem::Platform::RUBY
  s.authors     = ["Klaus Kämpf"]
  s.email       = ["kkaempf@suse.de"]
  s.homepage    = "http://www.github.com/openwsman/openwsman"
  s.summary = "Ruby client bindings for Openwsman"
  s.description = "The openwsman gem provides a Ruby API to manage
systems using the WS-Management protocol."

  s.required_rubygems_version = ">= 1.3.6"
  s.add_development_dependency("rake-compiler", [">= 0"])
  s.add_development_dependency("mocha", [">= 0.9"])
  s.add_development_dependency("yard", [">= 0.5"])
  s.extensions	<< "ext/openwsman/extconf.rb"

  s.files        = Dir.glob("lib/**/*.rb") +
                   Dir.glob("ext/**/*.{h,c}") +
                   Dir.glob("ext/*.{h,c}")

#  s.require_path = ''

  s.post_install_message = <<-POST_INSTALL_MESSAGE
  ____
/@    ~-.
\/ __ .- | remember to have fun! 
 // //  @  

  POST_INSTALL_MESSAGE
end

