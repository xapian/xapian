#!/usr/bin/perl -w
require 5.003;
use strict;

my %tests = (
'B financi compani AND report OR dolli OR' => 5773
);

for my $args (sort keys %tests) {
   open PIPE, "./matchtest $args|" or die $!;
   my $mtotal;
   while (<PIPE>) {
      if (/msize = \d+, mtotal = (\d+)/) {
         $mtotal = $1;
      }
   }
   close PIPE;
   if ($mtotal != $tests{$args}) {
      print "Got $mtotal, expected $tests{$args} - $args";
   }
}
