DIR = File.dirname(__FILE__)

$: << DIR

# Autotools binary
$:.unshift File.expand_path(File.join(DIR,"../.libs"))

# cmake binary
$:.unshift File.expand_path(File.join(DIR,"../../../build/bindings/ruby"))

# cmake local for openwsman.rb
$:.unshift File.expand_path(File.join(DIR,".."))
