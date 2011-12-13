$: << File.dirname(__FILE__)

# Autotools binary
$:.unshift "../.libs"

# cmake binary
$:.unshift "../../../build/bindings/ruby"

# cmake local for openwsman.rb
$:.unshift ".."
