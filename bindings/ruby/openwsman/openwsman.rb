# This is openwsman/openwsman
# do NOT require this file, but do a simple
#  require 'openwsman'
# instead
#

require 'openwsman/version'

# this loads the binary .so file
require '_openwsman'

# this extends Openwsman::XmlNode with method_missing
require 'openwsman/xmlnode'

# this extends Openwsman::XmlDoc with method_missing
require 'openwsman/xmldoc'

module Openwsman
  class Transport
    # called when authentication credentials missing or wrong
    def Transport.auth_request_callback client, auth_type
      # override in client code
      # return Array of [ username, password ]
      # return nil to abort authentication
    end
  end
  #
  # return endpoint-reference (EPR) prefix for given classname and namespace
  #
  # * +classname+ - classname (using the <schema>_<name> format)
  # * +namespace+ - optional namespace, required for Windows WMI which embeds the namespace in the EPR
  #
  # ==== Examples
  #   prefix = Openwsman.epr_prefix_for "CIM_Managed_Element"
  #   prefix = Openwsman.epr_prefix_for "Win32_Foo", "root/cimv2"
  #
  def self.epr_prefix_for classname, namespace = nil
    prefix = Openwsman::uri_prefix classname
    prefix += "/#{namespace}" if namespace && !namespace.empty?
    prefix
  end
  
  # create full endpoint reference URI for namespace and classname
  def self.epr_uri_for namespace, classname
    raise "Namespace must not be nil" unless namespace
    raise "Classname must not be nil" unless classname
    epr = "#{self.epr_prefix_for(classname)}"
    epr << "/#{namespace}" unless namespace.empty?
    epr << "/#{classname}"
  end
  
  class EndPointReference
    def method_missing name, *args
      selector(name)
    end
  end
end
