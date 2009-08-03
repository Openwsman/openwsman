#!/usr/bin/perl

use lib '../../../build/bindings/perl';
use lib '..';

use openwsman;

# host, port, path, scheme, username, password
$self->{CLIENT} = new openwsman::Client::('localhost', 8889, '/wsman', 'http', 'wsman', 'secret') or die print '<span class="error">[ERROR] Could not create client handler.</span><br/>';

$self->{CLIENT_OPTIONS} = new openwsman::ClientOptions::() or die print '<span class="error">[ERROR] Could not create client options handler.</span><br/>';

my $doc = $self->{CLIENT}->identify($self->{CLIENT_OPTIONS}) or die print '<span class="error">[ERROR] Could not connect to server.</span><br/>';
my $root = $doc->root;
my $prot_version = $root->find($openwsman::XML_NS_WSMAN_ID, "ProtocolVersion")->text();
my $prod_vendor = $root->find($openwsman::XML_NS_WSMAN_ID, "ProductVendor")->text();
my $prod_version = $root->find($openwsman::XML_NS_WSMAN_ID, "ProductVersion")->text();

print "Protocol $prot_version, Vendor $prod_vendor, Version $prod_version\n";
