#
# Do parallel enumerate request via celluloid gem
#
# Tests Ruby 1.9 threading model and proper use of rb_thread_blocking_region by the bindings
#
# Written by https://github.com/thehappycoder
#
require 'openwsman'
require 'celluloid'
require 'open-uri'

connection_credentials = [
    {
        :host => '10.120.67.93',
        :port => 5985,
        :username => 'wsman',
        :password => 'secret'
    },

    {
        :host => '10.120.4.11',
        :port => 5985,
        :username => 'wsman',
        :password => 'secret'
    }
]

uris = [
#    'http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/root/dcim/DCIM_NICView'
    'http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_ComputerSystem'
    #'http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/root/dcim/DCIM_SoftwareIdentity'
]

class Downloader
  include Celluloid

  def download(host, port, username, password, uri)
    puts "Requesting #{host}/#{File.basename(uri)}"
    @wsman = Openwsman::Client.new(host, port, "/wsman", "http", username, password)
    #Openwsman::debug = 9
    @wsman.transport.verify_peer=0
    @options = Openwsman::ClientOptions.new
    @options.flags = Openwsman::FLAG_ENUMERATION_OPTIMIZATION
    @options.max_elements = 100

    puts "Client #{host} prepared"
    request(uri) do |response|
      puts "#{host} responded"
    end
  end

  def download_http(*params)
    host = 'http://google.com'
    puts "Requesting #{host}"
    open(host).read
    puts "#{host} responded"
  end

  private
  def request(uri, &response_handler_block)
    puts "Enumerate..."
    response = @wsman.enumerate(@options, nil, uri)
    puts "...done"
    response_handler_block.call(response)

    context = response.context
    puts "Context #{context}"

    while (context)
      response = @wsman.pull( @options, nil, uri, context )
      response_handler_block.call(response)
      context = response.context
    end
  end
end

downloaders = Downloader.pool(size: connection_credentials.size * uris.size)

connection_credentials.each do |credential|
  uris.each do |uri|
    puts "Starting async"
    downloaders.async.download(credential[:host], credential[:port], credential[:username], credential[:password], uri)
  end
end

sleep
