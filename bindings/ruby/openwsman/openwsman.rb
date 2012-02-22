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
  # return EPR prefix for given classname (<schema>_<classname>)
  #
  def self.epr_prefix_for classname, namespace = nil
    schema = classname.split("_")[0] rescue nil
    return case schema
    # dmtf CIM
    when "CIM" then "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2"
    # dmtf reserved
    when "PRS" then "http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2"
    # Microsoft WMI
    when "Win32"
      prefix = "http://schemas.microsoft.com/wbem/wsman/1/wmi"
      prefix += "/#{namespace}" if namespace
      prefix
    # openwbem.org
    when "OpenWBEM" then "http://schema.openwbem.org/wbem/wscim/1/cim-schema/2"
    # sblim
    when "Linux" then "http://sblim.sf.net/wbem/wscim/1/cim-schema/2"
    # omc-project
    when "OMC" then "http://schema.omc-project.org/wbem/wscim/1/cim-schema/2"
    # pegasus.org
    when "PG" then "http://schema.openpegasus.org/wbem/wscim/1/cim-schema/2"
    # Intel AMT
    when "AMT" then "#http://intel.com/wbem/wscim/1/amt-schema/1"
    # Intel
    when "IPS" then "#http://intel.com/wbem/wscim/1/ips-schema/1"
    else
      raise "Unsupported schema #{schema.inspect} of class #{classname.inspect}"
    end
  end
end
