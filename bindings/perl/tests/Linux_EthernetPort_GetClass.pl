#!/usr/bin/perl -w

#
# perl example to get Linux_EthernetPort class info.
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

my $uri = $openwsman::XML_NS_CIM_INTRINSIC.'/Linux_EthernetPort'; # Uri.
my $method = $openwsman::CIM_ACTION_GET_CLASS; # GetClass Method.
my $result; # Used to store obtained data.

# Get class.
# (options, resource uri)
$result = $client->invoke($options, $uri, $method);
unless($result && $result->is_fault eq 0) {
    die print "[ERROR] Could not get class.\n";
}
my $classInfo = $result->body()->child();

# Get items.
my $name; # Class name.
my @qualifiers; # Class qualifiers.
my @properties; # Class properties.
for((my $cnt = 0) ; ($cnt<$classInfo->size()) ; ($cnt++)) {
    if($classInfo->get($cnt)->name() eq 'name') { # Class name.
        $name = $classInfo->get($cnt)->text();

    } elsif($classInfo->get($cnt)->name() eq 'qualifiers') { # Qualifiers.
        my $qualifNode = $classInfo->get($cnt); # Node which contains qualifiers.

        # Loop qualifiers.
        for((my $cnt4 = 0) ; ($cnt4<$qualifNode->size()) ; ($cnt4++)) {
            my $qualif;
            $qualif->{$qualifNode->get($cnt4)->name()} = $qualifNode->get($cnt4)->text();
            push @qualifiers, $qualif;
        }
        
    } elsif($classInfo->get($cnt)->name() eq 'properties') { # Properties.
        my $propsNode = $classInfo->get($cnt); # Node which contains properties.

        # Loop properties.
        for((my $cnt2 = 0) ; ($cnt2<$propsNode->size()) ; ($cnt2++)) {
            my $propNode = $propsNode->get($cnt2); # Property node.
            my $prop;

            for((my $cnt3 = 0) ; ($cnt3<$propNode->size()) ; ($cnt3++)) {
                if($propNode->get($cnt3)->name() eq 'name') { # Name.
                    $prop->{'name'} = $propNode->get($cnt3)->text();
                    
                } elsif ($propNode->get($cnt3)->name() eq 'type') { # Type.
                    $prop->{'type'} = $propNode->get($cnt3)->text();
                    
                } elsif ($propNode->get($cnt3)->name() eq 'value') { # Default Value.
                    $prop->{'value'} = $propNode->get($cnt3)->text();
                    
                } elsif ($propNode->get($cnt3)->name() eq 'qualifiers') { # Qualifiers.

                    # Loop properties qualifiers.
                    for((my $cnt4 = 0) ; ($cnt4<$propNode->get($cnt3)->size()) ; ($cnt4++)) {
                        my $propQual = $propNode->get($cnt3)->get($cnt4); # Property Qualifier node.

                        for((my $cnt5 = 0) ; ($cnt5<$propQual->size()) ; ($cnt5++)) {
                            if($propQual->get($cnt5)->name() eq 'type') {
                                $prop->{'qualifiers'}->{$propQual->get(0)->text()}->{'type'} =
                                    $propQual->get($cnt5)->text();
                            } elsif($propQual->get($cnt5)->name() eq 'value') {
                                push @{$prop->{'qualifiers'}->{$propQual->get(0)->text()}->{'value'}},
                                $propQual->get($cnt5)->text();
                            }
                        }
                    }
                }
            }

            push @properties, $prop;
        }
    }
}

# Print output.
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
    my %inf = %{$_}; # Info container.
    print "\tName: ", $inf{'name'}, "\n";
    print "\tType: ", $inf{'type'}, "\n";
    print "\tDefault Value: ", $inf{'value'}, "\n";
    print "\tQualifiers:\n";
    for my $key (keys %{$inf{'qualifiers'}}) {
        my %qualif = %{$inf{'qualifiers'}->{$key}};
        print "\t\tName: ", $key, "\n";
        print "\t\tType: ", $qualif{'type'}, "\n";
        print "\t\tValue/s: | ";
        foreach(@{$qualif{'value'}}) {
            print $_, ' | ';
        }
        print "\n\n";
    }
    print "\n";
}
print "---------------------------------------------------\n";
