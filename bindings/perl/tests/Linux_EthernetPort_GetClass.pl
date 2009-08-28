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

my $uri = $openwsman::XML_NS_CIM_INTRINSIC.'/Linux_EthernetPort'; # Uri.
my $method = $openwsman::CIM_ACTION_GET_CLASS; # GetClass Method.
my $result; # Used to store obtained data.

# Get class.
# (options, resource uri)
$result = $client->invoke($options, $uri, $method);
unless($result) {
    die print "<span class=\"error\">[ERROR] Could not get class.</span><br/>";
}
my $classInfo = $result->body()->child();

# Get items.
my $name; # Class name.
my @qualifiers; # Class qualifiers.
my @properties; # Class properties.
for((my $cnt = 0) ; ($cnt<$classInfo->size()) ; ($cnt++)) {
    if($classInfo->get($cnt)->name() eq 'name') {
        $name = $classInfo->get($cnt)->text();
        next;
    }

    if($classInfo->get($cnt)->name() eq 'qualifiers') {
        my $qualifNode = $classInfo->get($cnt); # Node which contains qualifiers.

        # Loop qualifiers.
        for((my $cnt4 = 0) ; ($cnt4<$qualifNode->size()) ; ($cnt4++)) {
            my $qualif;
            $qualif->{$qualifNode->get($cnt4)->name()} = $qualifNode->get($cnt4)->text();
            push @qualifiers, $qualif;
        }
        next;
    }

    if($classInfo->get($cnt)->name() eq 'properties') {
        my $propsNode = $classInfo->get($cnt); # Node which contains properties.

        # Loop properties.
        for((my $cnt2 = 0) ; ($cnt2<$propsNode->size()) ; ($cnt2++)) {
            my $propNode = $propsNode->get($cnt2); # Property node.
            my $prop;
            $prop->{'name'} = $propNode->get(0)->text();
                      
            # Loop properties qualifiers.
            for((my $cnt3 = 0) ; ($cnt3<$propNode->get(1)->size()) ; ($cnt3++)) {
                $prop->{'qualifiers'}->{$propNode->get(1)->get($cnt3)->name()} =
                    $propNode->get(1)->get($cnt3)->text();
            }

            push @properties, $prop;
        }
        next;
    }
}

print "---------------------------------------------------\n";
print 'Class Name: ', $name, "\n";
print "Class Qualifiers:\n";
foreach(@qualifiers) {
    my %inf = %$_; # Info container.
    while ((my $key, my $value) = each %inf) {
        print "\t", $key, ': ', $value, "\n\n";
    }
}
print "Class Properties: \n";
foreach(@properties) {
    my %inf = %$_; # Info container.
    print "\tName: ", $inf{'name'}, "\n";
    print "\tQualifiers:\n";
    for my $key (keys %inf) {
        if($key eq 'name') {
            next;
        }
        if($key eq 'qualifiers') {
            my %qualif = %{$inf{$key}};
            for my $key2 (keys %qualif) {
                print "\t\t", $key2,': ', $qualif{$key2}, "\n";
            }
        }
    }
    print "\n";
}
print "---------------------------------------------------\n";
