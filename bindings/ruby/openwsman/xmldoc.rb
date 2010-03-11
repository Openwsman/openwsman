#
# Assume XmlDoc.foo means XmlDoc.body.foo
#

module Openwsman
  class XmlDoc
    def method_missing method, *args
      self.body.send method,*args
    end
  end
end
