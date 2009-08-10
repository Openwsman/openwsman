#!/usr/bin/perl -w

use strict;
use warnings;
use lib '../../../build/bindings/perl';
use lib '..';
use openwsman;

# debug (to stderr)
# openwsman::set_debug(1);

# Create client instance.
# (host, port, path, scheme, username, password)
my $client = new openwsman::Client::('localhost', 5985, '/wsman', 'http', 'wsman', 'secret')
    or die print "[ERROR] Could not create client handler.\n";

# Alternate way.
# my $client = new openwsman::Client::('http://wsman:secret@localhost:8889/wsman')
#  or die print "[ERROR] Could not create client handler.\n";

# Set up client options.
my $options = new openwsman::ClientOptions::()
    or die print "[ERROR] Could not create client options handler.\n";

# Force basic auth.
$client->transport()->set_auth_method($openwsman::BASIC_AUTH_STR);

my $uri = 'http://sblim.sf.net/wbem/wscim/1/cim-schema/2/Linux_EthernetPort'; # Uri.
my @selectors = (
    ['DeviceID', 'eth0'],
    ['CreationClassName', 'Linux_EthernetPort'],
    ['SystemCreationClassName', 'Linux_ComputerSystem'],
    ['SystemName', 'localhost.localdomain']); # Selectors list.
my $result; # Used to store obtained data.
my @inst; # Instance.

# Establishing selectors.
# (key, value)
for(my $i=0 ; $i<scalar(@selectors) ; $i++) {
    $options->add_selector($selectors[$i][0],
                           $selectors[$i][1]);
}

# Get instance.
# (options, resource uri)
$result = $client->get($options, $uri);
unless($result) {
    die print "<span class=\"error\">[ERROR] Could not get instance.</span><br/>";
}
my $info = $result->body()->child();

# Get items.
my $items;
for((my $cnt = 0) ; ($cnt<$info->size()) ; ($cnt++)) {
    $items->{$info->get($cnt)->name()} = $info->get($cnt)->text();
}
push @inst, $items;

print "-----------------\n";
foreach(@inst) {
    my %inf = %$_; # Info container.
    for my $key (keys %inf) {
        print $key,': ',$inf{$key},"\n";
    }
}
print "-----------------\n";
