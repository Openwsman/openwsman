#!/usr/bin/perl

use lib '../../../build/bindings/perl';
use lib '..';

use openwsman;

# debug (to stderr)
# openwsman::set_debug(-1);

# host, port, path, scheme, username, password
$self->{CLIENT} = new openwsman::Client::('localhost', 5985, '/wsman', 'http', 'wsman', 'secret') or die print '<span class="error">[ERROR] Could not create client handler.</span><br/>';

# alternate way 
# $self->{CLIENT} = new openwsman::Client::('http://wsman:secret@localhost:8889/wsman') or die print '<span class="error">[ERROR] Could not create client handler.</span><br/>';

$self->{CLIENT_OPTIONS} = new openwsman::ClientOptions::() or die print '<span class="error">[ERROR] Could not create client options handler.</span><br/>';

# Dump the XML request to stdout
# $self->{CLIENT_OPTIONS}->set_dump_request();

# force basic auth
$self->{CLIENT}->transport()->set_auth_method($openwsman::BASIC_AUTH_STR);

my $doc = $self->{CLIENT}->identify($self->{CLIENT_OPTIONS}) or die print '<span class="error">[ERROR] Could not connect to server.</span><br/>';
my $root = $doc->root;
my $prot_version = $root->find($openwsman::XML_NS_WSMAN_ID, "ProtocolVersion")->text();
my $prod_vendor = $root->find($openwsman::XML_NS_WSMAN_ID, "ProductVendor")->text();
my $prod_version = $root->find($openwsman::XML_NS_WSMAN_ID, "ProductVersion")->text();

print "Protocol $prot_version, Vendor $prod_vendor, Version $prod_version\n";
