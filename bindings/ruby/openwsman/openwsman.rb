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

# = About openwsman
# Openwsman (http://www.openwsman.org) is a project intended to provide an open-source
# implementation of the Web Services Management specification
# (WS-Management) and to expose system management information on the
# Linux operating system using the WS-Management protocol. WS-Management
# is based on a suite of web services specifications and usage
# requirements that exposes a set of operations focused on and covers
# all system management aspects. 
#
# = Using the bindings
# The bindings provide access to the client-side API of openwsman.
# You start by creating a Client instance and set up ClientOptions
# to control the communication.
#
# The Client instance now provides the WS-Management operations, like
# enumerate, get, invoke, etc.
#
# All client operations return a XmlDoc representing the SOAP response
# from the system.
# # You can then use XmlDoc methods to extract SOAP elements from the
# response and dig down through its XmlNode and XmlAttr objects.

module Openwsman
  #
  # ClientOptions
  #
  class ClientOptions
    # assign hash to properties
    def properties= value
      value.each do |k,v|
        self.add_property k.to_s, v
      end
    end
    # assign hash to selectors
    def selectors= value
      value.each do |k,v|
        self.add_selector k.to_s, v
      end
    end
  end
  #
  # Transport
  #
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
  #   => "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2"
  #   prefix = Openwsman.epr_prefix_for "Win32_Foo", "root/cimv2"
  #   => "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2"
  #
  def self.epr_prefix_for classname, namespace = nil
    prefix = Openwsman::uri_prefix classname
    prefix += "/#{namespace}" if namespace && !namespace.empty?
    prefix
  end
  
  # create full endpoint reference URI for namespace and classname
  #
  # * +classname+ - classname (using the <schema>_<name> format)
  # * +namespace+ - optional namespace, required for Windows WMI which embeds the namespace in the EPR
  #
  # ==== Examples
  #   Openwsman.epr_uri_for "root/cimv2", "Win32_Foo"
  #   => "http://schemas.microsoft.com/wbem/wsman/1/wmi/root/cimv2/Win32_Foo"
  def self.epr_uri_for namespace, classname
    raise "Namespace must not be nil" unless namespace
    raise "Classname must not be nil" unless classname
    epr = epr_prefix_for(classname,namespace) + "/#{classname}"
  end
  #
  # EndPointReference
  #
  class EndPointReference
    def method_missing name, *args # :nodoc:
      selector(name.to_s)
    end
    # convert EndPointReference to string
    def to_s
      s = "#{classname}"
      first = true
      self.each do |k,v|
        s << ((first)?"?":"&")
        first = false
        s << "#{k}=#{v.inspect}"
      end
      s
    end
  end
  #
  # Fault
  #
  class Fault
    # convert Fault to string
    def to_s
      "Fault #{code}.#{subcode}: #{detail} - #{reason}"
    end
  end
end
