#
# openwsmanplugin.rb
#
# Generic openwsman server plugin
#

module Openwsman
  class XmlDoc
    def self.method_missing method, *args
      STDERR.puts "#{self.class}.#{method} not implemented"
    end
  end
  class XmlNode
    def self.method_missing method, *args
      STDERR.puts "#{self.class}.#{method} not implemented"
    end
  end
  def self.create_plugin
    Sample.new
  end
  #
  # identify
  #
  def self.identify context
    STDERR.puts "OpenwsmanPlugin.identify, context #{context}"
  end
  #
  # enumerate
  #
  def self.enumerate context, enum_info, status
    STDERR.puts "OpenwsmanPlugin.enumerate, context #{context}, enum_info #{enum_info}, status #{status}"
    STDERR.puts "to #{enum_info.epr_to}, uri #{enum_info.epr_uri}"
    STDERR.puts "class #{context.classname}, method #{context.method}"
    STDERR.puts "action #{context.action}, resource_uri #{context.resource_uri}"
    selectors = context.selectors
    STDERR.puts "selectors #{selectors}"
    
    enum_info.index = 0;
		      
    enum_info.total_items = 10
    # error
    #   enumInfo.pull_result = nil
    # end
    enum_info.enum_results = 1
    enum_info.enum_context = 1
    true
  end
  #
  # pull
  #
  def self.pull context, enum_info, status
    STDERR.puts "OpenwsmanPlugin.pull, context #{context}, enum_info #{enum_info}, status #{status}"
    STDERR.puts "enum_info.index #{enum_info.index} enum_info.total_items #{enum_info.total_items}"
    if enum_info.index < enum_info.total_items then
      out_doc = context.indoc.create_response_envelope
      body = out_doc.body
      response = body.add(XML_NS_ENUMERATION, WSENUM_PULL_RESP)
      response = response.add(XML_NS_ENUMERATION, WSENUM_ITEMS)
      response.add(nil, "item#{enum_info.index}", "#{enum_info.index}")
      STDERR.puts "pull response #{out_doc}"
      enum_info.pull_result = out_doc
      return true
    else
      enum_info.enum_results = nil
      enum_info.enum_context = nil
      STDERR.puts "pull failed"
    end
  end
  #
  # release
  #
  def self.release context, enum_info, status
    STDERR.puts "OpenwsmanPlugin.release, context #{context}"
    true
  end
  #
  # create
  #
  def self.create op
    STDERR.puts "OpenwsmanPlugin.create, op #{op}"
  end
  #
  # delete
  #
  def self.delete op
    STDERR.puts "OpenwsmanPlugin.delete, op #{op}"
  end
  #
  # get
  #
  def self.get op
    STDERR.puts "OpenwsmanPlugin.get, op #{op}"
    soap = op.soap
    STDERR.puts "soap #{soap}"
    indoc = op.indoc
    STDERR.puts "indoc #{indoc}"
    context = soap.create_ep_context(indoc)
    STDERR.puts "context #{context}"
    # retrieve custom action
    msg = op.msg
    STDERR.puts "msg #{msg}"
    selectors = context.selectors
    STDERR.puts "selectors #{selectors}"
    uri = context.resource_uri
    STDERR.puts "uri #{uri}"
	    
    op.outdoc = indoc.create_response_envelope
    body = op.outdoc.body
    response = body.add(XML_NS_TRANSFER, TRANSFER_GET_RESP)
    response = response.add(XML_NS_TRANSFER, "foo", "bar")
    true
  end
  #
  # put
  #
  def self.put op
    STDERR.puts "OpenwsmanPlugin.put, op #{op}"
  end
  #
  # custom
  #
  def self.custom op
    STDERR.puts "OpenwsmanPlugin.custom, op #{op}"
    status.code = WSA_ENDPOINT_UNAVAILABLE
    status.detail = WSMAN_DETAIL_LOCALE
    status.msg = "This Ruby plugin does not implement 'get'"
    doc = XmlDoc.new "fault"
    status.generate_fault(doc)
  end
  #
  # catch missing methods
  #
  def self.method_missing method, *args
    STDERR.puts "#{self.class}.#{method} not implemented"
  end
  class Sample
    SCHEMA = "http://schema.opensuse.org/swig/wsman-schema/1-0"
    def initialize *args
      STDERR.puts "OpenwsmanPlugin.new #{args}"
    end
    def namespaces
      [ [SCHEMA, "Ruby"] ]
    end
    def identify context
      STDERR.puts "OpenwsmanPlugin.identify, context #{context}"
    end
    def enumerate context, enum_info, status
      
      STDERR.puts "OpenwsmanPlugin.enumerate, context #{context}"
    end
    def release context, enum_info, status
      STDERR.puts "OpenwsmanPlugin.release, context #{context}"
    end
    def pull context, enum_info, status
      STDERR.puts "OpenwsmanPlugin.pull, op #{op}"
    end
    def get op
      STDERR.puts "OpenwsmanPlugin.get, op #{op}"
    end
    def custom op
      STDERR.puts "OpenwsmanPlugin.custom, op #{op}"
    end
    def put op
      STDERR.puts "OpenwsmanPlugin.put, op #{op}"
    end
    def create op
      STDERR.puts "OpenwsmanPlugin.create, op #{op}"
    end
    def delete op
      STDERR.puts "OpenwsmanPlugin.delete, op #{op}"
    end
  end
end
