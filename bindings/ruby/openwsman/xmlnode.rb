module Openwsman
  class XmlNode
    def method_missing method, *args # :nodoc:
      find(nil, method.to_s)
    end
  end
end
