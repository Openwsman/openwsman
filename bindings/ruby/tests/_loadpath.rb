DIR = File.dirname(__FILE__)

$: << DIR

if ENV["OPENWSMAN_GEM"] # set OPENWSMAN_GEM to test gem-based openwsman
  require 'rubygems'
else
  # Autotools binary
  $:.unshift File.expand_path(File.join(DIR,"../.libs"))

  # cmake binary
  $:.unshift File.expand_path(File.join(DIR,"../../../build/bindings/ruby"))

  # cmake local for openwsman.rb
  $:.unshift File.expand_path(File.join(DIR,".."))
end
