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
end