# How to compile and run openwsman from GIT ? 

After checking out the project from git, create a build directory
with

    mkdir build

cd into it

    cd build
    cmake ..
    make

See `package/openwsman.spec.in` for running cmake in 'release' mode

Most likely you will need to install some of the packages, depending
on the distribution you are running.

Pre-build (RPM) packages for many distributions are available at
[the openSUSE build service](https://build.opensuse.org/package/show/systemsmanagement:wbem/openwsman)


## Packages and other software needed:

* libxml2
* sblim-sfcc (from the sblim project)
* swig and python for python binding support
* other development packages like cmake, gcc, etc.

After all packages are installed, compile everything and install. The
server can run as a daemon, which would require root access; But it
can be run in the foreground with debugging messages printed to stdout
as well. This the help output when you run:

    /usr/sbin/openwsmand --help
    Usage:
      openwsmand [Option...] WS-Management Server
  
    Help Options
      -?, --help
    
    Application Options
      -S, --ssl                                       Use SSL port
      -q, --version                                   Display application version
      -d, --debug                                     Start daemon in foreground and turn on debugging
      -s, --syslog=0-6                                Set the verbosity of syslog output.
      -e, --enum-idle-timeout=default 100             Enumeration Idle timeout in secs
      -c, --config-file=<file>                        Alternate configuration file
      -p, --pid-file=<file>                           PID file
      -r, --subscription-repository-location=<uri>    Subscription Repository Location
      -a, --auth-dir=<directory>                      Authentication plugin dir (testing)
      -b, --basic-password-file=<file>                Basic password file (testing)
      -P, --plugin-dir=<directory>                    Dispatcher plugins directory (testing)

### Running with SSL enabled

openwsmand requires a certificate when running in SSL mode (`-S` option).
This can be created by running the `owsmangencert.sh` script available
in `./etc` (after build) resp. `/etc/openwsman` (after install).

## Configuration file

Starting from version 0.1.1 a configuration file is needed. you can
find an example in the `./etc` directory. The configuration file has the
following syntax:

    [server]
    port = 5985
    #ssl_port = 5986
    ssl_cert_file = /etc/openwsman/servercert.pem
    ssl_key_file = /etc/openwsman/serverkey.pem
    #digest_password_file = /etc/openwsman/digest_auth.passwd
    basic_password_file = /etc/openwsman/simple_auth.passwd
    enum_idle_timeout = 5000

    max_threads = 1

    #use_digest is OBSOLETED, see below.

    #
    # Authentication backend for BASIC authentication. Default is to read a configuration file defined with 'basic_password_file'
    #

    #basic_authenticator = libwsman_pam_auth.so
    #basic_authenticator_arg = openwsman


    [client]
    port = 5988
    agent = openwsman 2.2.4

    [cim]
    default_cim_namespace = root/cimv2

    # The following are in part fake namespaces for some publicly available CIM implementations.
    vendor_namespaces = OpenWBEM=http://schema.openwbem.org/wbem/wscim/1/cim-schema/2,Linux=http://sblim.sf.net/wbem/wscim/1/cim-schema/2,OMC=http://schema.omc-project.org/wbem/wscim/1/cim-schema/2,Reef=http://reef.sblim.sf.net/wbem/wscim/1/cim-schema/2,CWS=http://cws.sblim.sf.net/wbem/wscim/1/cim-schema/2

## Running openwsman server

To start the server in the foreground, run:

    /usr/sbin/openwsmand  -d

You can also specify the configuration file to be used on the command line using the -c option.

## Running openwsman client

`wsman` is the client command line tool for executing WSMAN requests against the openwsman server:

    /usr/bin/wsman --help-all

    Usage:
      wsman [Option...] <action> <Resource Uri>

    Help Options
     -?, --help
     --help-all                                      Show help options
     --help-enumeration                              Enumeration Options
     --help-tests                                    Test Cases
     --help-cim                                      CIM Options
     --help-flags                                    Request Flags
     --help-event                                    Subscription Options

    Enumeration
     -m, --max-elements=<max number of elements>     Max Elements Per Pull/Optimized Enumeration
     -o, --optimize                                  Optimize enumeration results
     -E, --estimate-count                            Return estimation of total items
     -M, --enum-mode=epr|objepr                      Enumeration Mode
     -U, --enum-context=<enum context>               Enumeration Context (For use with Pull and Release)

    Tests
     -f, --from-file=<file name>                     Send request from file
     -R, --print-request                             print request on stdout
     -Q, --request                                   Only output reqest. Not send it.
     -S, --step                                      Do not perform multiple operations (do not pull data when enumerating)

    CIM
     -N, --namespace=<namespace>                     CIM Namespace (default is root/cimv2)
     -B, --binding-enum-mode=none|include|exclude    CIM binding Enumeration Mode
     -T, --cim-extensions                            Show CIM Extensions
     -W, --references                                CIM References
     -w, --associators                               CIM Associators

    Flags
     -x, --filter=<filter>                           Filter
     -D, --dialect=<dialect>                         Filter Dialect
     -t, --operation-timeout=<time in sec>           Operation timeout in seconds
     -e, --max-envelope-size=<size>                  maximal envelope size
     -F, --fragment=<fragment>                       Fragment (Supported Dialects: XPATH)

    Event subscription
     -G, --delivery-mode=<mode>                      Four delivery modes available: push/pushwithack/events/pull
     -Z, --notification-uri=<uri>                    Where notifications are sent
     -r, --subscription-expiry-time=<seconds>        subscription expiry time in seconds
     -H, --heartbeat=<seconds>                       Send hearbeat in an interval
     -l, --bookmark                                  Send bookmark
     -i, --subscription-identifier=<uuid:XXX>        Used to specify which subscription
     -L, --notify-reference-properties=<xs:anyURI>   Notify Reference Properties

    Application Options
     -d, --debug=1-6                                 Set the verbosity of debugging output.
     -c, --cacert=<filename>                         Certificate file to verify the peer
     -A, --cert=<filename>                           Certificate file. The certificate must be in PEM format.
     -K, --sslkey=<key>                              SSL Key.
     -u, --username=<username>                       User name
     -g, --path=<path>                               Path
     -J, --input=<filename>                          File with resource for Create and Put operations in XML, can be a SOAP envelope
     -p, --password=<password>                       Password
     -h, --hostname=<hostname>                       Host name
     -b, --endpoint=<url>                            End point
     -P, --port=<port>                               Server Port
     -X, --proxy=<proxy>                             Proxy name
     -Y, --proxyauth=<proxyauth>                     Proxy user:pwd
     -y, --auth=<basic|digest|gss>                   Authentication Method
     -a, --method=<custom method>                    Method (Works only with 'invoke')
     -k, --prop=<key=val>                            Properties with key value pairs (For 'put', 'invoke' and 'create')
     -C, --config-file=<file>                        Alternate configuration file
     -O, --out-file=<file>                           Write output to file
     -V, --noverifypeer                              Not to verify peer certificate
     -v, --noverifyhost                              Not to verify hostname
     -I, --transport-timeout=<time in sec>           Transport timeout in seconds

To create a password file, use the `htpasswd` and `htdigest` utilities
from the [Apache](http://www.apache.org) distribution.

You can connect to the server with the following command, which is part of the DMTF WS-Management specification

    wsman identify -h <hostname> --port 5988 -u wsman --password secret


The above command should give the following result:


    <?xml version="1.0" encoding="UTF-8"?>
    <s:Envelope xmlns:s="http://www.w3.org/2003/05/soap-envelope" xmlns:wsmid="http://schemas.dmtf.org/wbem/wsman/identity/1/wsmanidentity.xsd">
     <s:Header/>
     <s:Body>
      <wsmid:IdentifyResponse>
       <wsmid:ProtocolVersion>http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd</wsmid:ProtocolVersion>
       <wsmid:ProductVendor>openwsman</wsmid:ProductVendor>
       <wsmid:ProductVersion>2.2.4</wsmid:ProductVersion>
      </wsmid:IdentifyResponse>
     </s:Body>
    </s:Envelope>
