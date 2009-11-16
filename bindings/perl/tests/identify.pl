#!/usr/bin/perl

#
# perl example to get openwsmand info.
# written by warptrosse@gmail.com
#

use strict;
use warnings;
use lib '../../../build/bindings/perl';
use lib '../.libs';
use lib '..';
use openwsman;

# debug (to stderr)
# openwsman::set_debug(-1);

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

# Dump the XML request to stdout.
# $options->set_dump_request();

# Force basic auth.
$client->transport()->set_auth_method($openwsman::BASIC_AUTH_STR);

my $result; # Used to store obtained data.

# Identify.
# (options)
$result = $client->identify($options)
unless($result && $result->is_fault eq 0) {
    die print "[ERROR] Could not identify server.\n";
}

# Get server info.
my $root = $doc->root;
my $prot_version = $root->find($openwsman::XML_NS_WSMAN_ID,
                               "ProtocolVersion")->text();
my $prod_vendor = $root->find($openwsman::XML_NS_WSMAN_ID,
                              "ProductVendor")->text();
my $prod_version = $root->find($openwsman::XML_NS_WSMAN_ID,
                               "ProductVersion")->text();

# Print output.
print "Protocol $prot_version, Vendor $prod_vendor, Version $prod_version\n";
