module Openwsman
  class XmlDoc
    #
    # Assume XmlDoc.foo means XmlDoc.body.foo
    #
    def method_missing method, *args # :nodoc:
      self.body.send method,*args
    end
  end
end
