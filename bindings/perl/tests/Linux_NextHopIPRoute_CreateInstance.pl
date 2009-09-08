#!/usr/bin/perl -w

#
# perl example to create a new Linux_NextHopIPRoute instance.
# written by warptrosse@gmail.com
#

use strict;
use warnings;
use lib '../../../build/bindings/perl';
use lib '..';
use openwsman;

# debug (to stderr)
# openwsman::set_debug(1);

# Create client instance.
# (host, port, path, scheme, username, password)
my $client = new openwsman::Client::('localhost', 5985, '/wsman', 'http',
                                     'wsman', 'secret')
    or die print "[ERROR] Could not create client handler.\n";

# Alternate way.
# my $client = new openwsman::Client::('http://wsman:secret@localhost:8889/wsman')
#  or die print "[ERROR] Could not create client handler.\n";

# Set up client options.
my $options = new openwsman::ClientOptions::()
    or die print "[ERROR] Could not create client options handler.\n";

# Force basic auth.
$client->transport()->set_auth_method($openwsman::BASIC_AUTH_STR);

my $uri = 'http://sblim.sf.net/wbem/wscim/1/cim-schema/2/Linux_NextHopIPRoute'; # Uri.
my @dataInfo = (
    ['AddressType'        , "1"],
    ['AdminDistance'      , "1"],
    ['Caption'            , "NextHop IPv4 route."],
    ['Description'        , "NextHop IPv4 route."],
    ['DestinationAddress' , "192.168.0.0"],
    ['DestinationMask'    , "24"],
    ['ElementName'        , "IPv4-192.168.0.0/24"],
    ['Generation'         , "1"],
    ['InstanceID'         , "localhost|192.168.0.0|24|254|2|0|"],
    ['IsStatic'           , "1"],
    ['OtherDerivation'    , ""],
    ['PrefixLength'       , ""],
    ['RouteDerivation'    , "3"],
    ['RouteMetric'        , "1"],
    ['TypeOfRoute'        , "2"]
    ); # Selectors list.
my $result; # Used to store obtained data.

# Get class name.
my $className;
if(($uri =~ /.\/(\w+)$/)) {
    $className = $1;
} else {
    print "[ERROR] Malformed uri.\n";
    return 1;
}

# Establish data.
# (key, value)
my $data = new openwsman::XmlDoc::($className);
my $root = $data->root();
$root->set_ns($uri);
for(my $i=0 ; $i<scalar(@dataInfo) ; $i++) {
    $root->add($uri, $dataInfo[$i][0], $dataInfo[$i][1]);
}

# Dump the XML request to stdout.
# $options->set_dump_request();

# Create instance.
# (options, uri, data, data length, encoding)
$result = $client->create($options, $uri, $data->string(),
                          length($data->string()),"utf-8");
unless($result && $result->is_fault eq 0) {
    print "[ERROR] Could not create instance.\n";
} else {
    # Print output.
    print "---------------------------------------------------\n";
    print "Result: \n\n", $result->string();
    print "---------------------------------------------------\n";
}
