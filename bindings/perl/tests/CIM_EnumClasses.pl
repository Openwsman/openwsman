#!/usr/bin/perl -w

#
# perl example to enumerate CIMOM classes names.
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

my $uri = $openwsman::XML_NS_CIM_INTRINSIC; # Uri.
my $method = $openwsman::CIM_ACTION_ENUMERATE_CLASS_NAMES; # GetClass Method.
my $result; # Used to store obtained data.

# Get classes.
# (options, resource uri)
$result = $client->invoke($options, $uri, $method);
unless($result && $result->is_fault eq 0) {
    die print "[ERROR] Could not get classes.\n";
}
my $classesInfo = $result->body()->child();

# Get items.
my @classes; # Classes list.
for((my $cnt = 0) ; ($cnt<$classesInfo->size()) ; ($cnt++)) {
    push @classes, $classesInfo->get($cnt)->text();
}

print "---------------------------------------------------\n";
print "Classes names:\n\n";
foreach(@classes) {
    print $_, "\n";
}
print "---------------------------------------------------\n";
