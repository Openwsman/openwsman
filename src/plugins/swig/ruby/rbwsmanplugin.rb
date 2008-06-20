#
# rbwsmanplugin.rb
#
# Generic openwsman server plugin
#

class WsmanPlugin
  SCHEMA = "http://schema.opensuse.org/swig/wsman-schema/1-0"
  def initialize *args
    STDERR.puts "WsmanPlugin.new #{args}"
  end
  def namespaces
    [ [SCHEMA, "Ruby"] ]
  end
  def identify
    STDERR.puts "WsmanPlugin.identify"
    0
  end
  def enumerate
    STDERR.puts "WsmanPlugin.enumerate"
    0
  end
  def release
    STDERR.puts "WsmanPlugin.release"
    0
  end
  def pull
    STDERR.puts "WsmanPlugin.pull"
    0
  end
  def get
    STDERR.puts "WsmanPlugin.get"
    0
  end
  def custom
    STDERR.puts "WsmanPlugin.custom"
    0
  end
  def put
    STDERR.puts "WsmanPlugin.put"
    0
  end
  def create
    STDERR.puts "WsmanPlugin.create"
    0
  end
  def delete
    STDERR.puts "WsmanPlugin.delete"
    0
  end
end
