#!/usr/bin/perl -w
require 5.003;
use strict;

my $verbose = 0;
if ($ARGV[0] =~ /^--?v(?:erbose)$/) {
   $verbose = 1;
}

my %tests = (
'B financial company AND report OR dolly OR' => 5772,
'finance company report dolly' => 26427,
'olly betts zebra' => 20,
'olly betts zebra business news report financial' => 30248
);

my $fail = 0;
for my $args (sort keys %tests) {
   open PIPE, "./matchtest $args|" or die $!;
   my $mtotal;
   print "=" x 75, "\n" if $verbose;
   while (<PIPE>) {
      print if $verbose;
      if (/msize = \d+, mtotal = (\d+)/) {
         $mtotal = $1;
      }
   }
   close PIPE;
   print "=" x 75, "\n" if $verbose;
   if ($mtotal != $tests{$args}) {
      print "Got $mtotal, expected $tests{$args} - $args\n";
      $fail++;
   }
}

if ($fail) {
   print "$fail test(s) failed\n";
   exit 1;
}

print "All tests passed\n";
