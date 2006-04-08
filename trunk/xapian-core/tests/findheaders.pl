#! /usr/bin/perl -w
# findheaders.pl: generate a list of all included headers from Makefile.in-s
# Note: we read them from Makefile.in rather than Makefile.am so that automake
# has unwrapped lines for us
require 5.000;
use strict;

foreach (find_headers($ARGV[0] || ".")) {
   print "$_\n";
}

sub find_headers {
   my $dir = shift;
   # skip directory if it doesn't use automake
   return () unless -f "$dir/Makefile.am";
   open F, "<$dir/Makefile.in" or die "Makefile.in not present";
   my %h = ();
   my @subs = ();
   while (<F>) {
       while (s/\\\n//) {
           my $l = <F>;
	   last unless defined $l;
	   $_ .= $l;
       }
       if (s/^[A-Za-z0-9_]+_HEADERS\s*=\s*//) {
           foreach (map "$dir/$_", split /\s+/) {
               $h{$_}++;
	   }
       } elsif (s/^DIST_SUBDIRS\s*=\s*//) {
           push @subs, grep {$_ ne '.'} split /\s+/;
       }
   }
   close F;
   my @h = sort keys %h;
   undef %h;
   foreach my $d (sort @subs) {
       push @h, find_headers("$dir/$d");
   }
   return @h;
}
