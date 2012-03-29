# cimenum.rb
#  enumerate/pull/release for any class

require 'test/unit'
require 'rexml/document'
require File.join(File.dirname(__FILE__),'_loadpath')
require 'openwsman'
require '_client'

def show_fault result
  return unless result.fault?
  fault = Openwsman::Fault.new result
  puts "Fault code #{fault.code}, subcode #{fault.subcode}"
  puts "\treason #{fault.reason}"
  puts "\tdetail #{fault.detail}"
  exit 1
end

debug = nil
namespace = "root/cimv2"
classname = nil

loop do
  opt = ARGV.shift
  break unless opt
  case opt
  when "-d" then debug = true
  when "-n" then namespace = ARGV.shift
  else
    classname = opt
  end
end

raise "No classname given" unless classname
client = Client.open
options = Openwsman::ClientOptions.new

uri = Openwsman.epr_uri_for namespace, classname
result = client.enumerate( options, nil, uri )

raise "Connection failed" unless result

STDERR.puts result.to_xml if debug

show_fault result

count = 0
context = result.context
while (context)
  count += 1
  result = client.pull( options, nil, uri, context )
#      puts "Response ##{count}"
#      puts "### #{result.to_xml}"
  result.body.PullResponse.Items.each do |child|
    puts "#{child.name}"
    child.each do |prop|
      puts "\t#{prop.name} = #{prop.text}"
    end
  end
  context = result.context
end
client.release( options, uri, context ) if context
puts "Got #{count} responses"
