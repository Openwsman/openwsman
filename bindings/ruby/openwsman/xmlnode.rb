module Openwsman
  class XmlNode
    def method_missing method, *args
      find(nil, method.to_s)
    end
  end
end
