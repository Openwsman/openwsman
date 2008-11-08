#
# rbwsmanplugin.rb
#
# Generic openwsman server plugin
#

module Rbwsman
  def self.create_plugin
    Sample.new
  end
  class Sample
    SCHEMA = "http://schema.opensuse.org/swig/wsman-schema/1-0"
    def initialize *args
      STDERR.puts "WsmanPlugin.new #{args}"
    end
    def namespaces
      [ [SCHEMA, "Ruby"] ]
    end
    def identify context
      STDERR.puts "WsmanPlugin.identify, context #{context}"
    end
    def enumerate context, enum_info, status
      
      STDERR.puts "WsmanPlugin.enumerate, context #{context}"
    end
    def release context, enum_info, status
      STDERR.puts "WsmanPlugin.release, context #{context}"
    end
    def pull context, enum_info, status
      STDERR.puts "WsmanPlugin.pull, op #{op}"
    end
    def get op
      STDERR.puts "WsmanPlugin.get, op #{op}"
    end
    def custom op
      STDERR.puts "WsmanPlugin.custom, op #{op}"
    end
    def put op
      STDERR.puts "WsmanPlugin.put, op #{op}"
    end
    def create op
      STDERR.puts "WsmanPlugin.create, op #{op}"
    end
    def delete op
      STDERR.puts "WsmanPlugin.delete, op #{op}"
    end
end
end
