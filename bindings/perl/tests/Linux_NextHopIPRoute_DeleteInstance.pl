#!/usr/bin/perl -w

#
# perl example to delete a specified Linux_NextHopIPRoute instance.
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
my @selectors = (
    ['InstanceID', "localhost|192.168.0.0|24|254|2|0|"]
    ); # Selectors list.
my $result; # Used to store obtained data.

# Establish selectors.
# (key, value)
for(my $i=0 ; $i<scalar(@selectors) ; $i++) {
    $options->add_selector($selectors[$i][0],
                           $selectors[$i][1]);
}

# Dump the XML request to stdout.
$options->set_dump_request();

# Delete instance.
# (options, uri)
$result = $client->delete($options, $uri);
unless($result && $result->is_fault eq 0) {
    print "[ERROR] Could not delete instance.\n";
}
