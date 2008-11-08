# rbwsman.rb
#
# wrapper to load rbwsman.so either from autotools build dir or from CMake build dir
#
if File.directory?('../.libs')
  require '../.libs/rbwsman'
elsif File.directory?('../../../build/bindings/ruby')
  require '../../../build/bindings/ruby/rbwsman'
else
  raise "rbwsman.so not found"
end
