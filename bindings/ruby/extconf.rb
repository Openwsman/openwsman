#
# extconf.rb for openwsman Gem
#

require 'mkmf'
# $CFLAGS = "#{$CFLAGS} -Werror"

# requires wsman, wsman_client, and libxml2

unless have_library('wsman', 'wsman_create_doc')
  STDERR.puts "Cannot find wsman_create_doc() in libwsman"
  STDERR.puts "Is openwsman-devel installed ?"
  exit 1
end
find_header 'wsman-xml-api.h', '/usr/include/openwsman'

unless have_library('wsman_client', 'wsmc_create')
  STDERR.puts "Cannot find wsmc_create() in libwsman_client"
  STDERR.puts "Is openwsman-devel installed ?"
  exit 1
end
find_header 'wsman-client-api.h', '/usr/include/openwsman'

unless have_library('xml2', 'xmlNewDoc')
  STDERR.puts "Cannot find xmlNewDoc() in libxml2"
  STDERR.puts "Is libxml2-devel installed ?"
  exit 1
end
find_header 'libxml/parser.h', '/usr/include/libxml2'

swig = find_executable("swig")
raise "SWIG not found" unless swig

major, minor, path = RUBY_VERSION.split(".")
raise "SWIG failed to run" unless system("#{swig} -ruby -autorename -DRUBY_VERSION=#{major}#{minor} -I. -I/usr/include/openwsman -o openwsman_wrap.c openwsman.i")

$CPPFLAGS = "-I/usr/include/openwsman -I.."

create_makefile('_openwsman')
