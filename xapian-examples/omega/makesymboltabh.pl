#!/usr/bin/perl -w
use strict;

open SYMTAB, ">symboltab.h" or die $!;
open SYMTABU, ">symboltab-unicode.h" or die $!;
open ENTTAB, ">entitytab.h" or die $!;
open ENTTABU, ">entitytab-unicode.h" or die $!;

my %ent = (
   'quot' => '"',
   'nbsp' => ' ',
   'lt' => '<',
   'gt' => '>'
);

my %entu = ();

while (<>) {
   next if /^\s*;/;
   if (/^#?(\d+)\s+(\S?)(\S?)(\S*)/) {
      if (length $4) {
         print STDERR "Transliteration > 2 chars: $_";
	 exit 1;
      }
      unless (length $2) {
         print STDERR "Transliteration empty: $_";
	 exit 1;
      }

      my ($uni, $ch, $cache) = ($1, $2, $3);
      # escape ''' and '\' to give '\'' and '\\' 
      $ch =~ s/\\/\\\\/g;
      $ch =~ s/'/\\'/g;
      if (length $cache) {
         $cache =~ s/\\/\\\\/g;
	 $cache =~ s/'/\'/g;
      }
      
      my $out = "case $uni: ch = '$ch'; ";
      $out .= "cache = '$cache'; " if length $cache;
      $out .= "break;\n";

      # fx only wants latin1, CFerret wants unicode too
      if ($uni < 256) {
         print SYMTAB $out;
      } else {
         print SYMTABU $out;
      }
   } elsif (/^(\S+)\s+#(\d+)/) {
      $entu{$1} = $2;
   } elsif (/^(\S+)\s+(\S?)(\S?)(\S*)/) {
      if (length $4) {
         print STDERR "Transliteration > 2 chars: $_";
	 exit 1;
      }
      unless (length $2) {
         print STDERR "Transliteration empty: $_";
	 exit 1;
      }
      $ent{$1} = "$2$3";   
   }
}

my $case;
my $first;

undef $case;
$first = 0;
foreach (sort keys %ent) {
   my ($newcase) = /(.)/;
   if (!defined $case || $case ne $newcase) {
      print ENTTAB " break;\n" if defined $case;
      $case = $newcase;      
      print ENTTAB "case '$case':\n";
      $first = 1;
   }
   if ($first) {
      $first = 0;
   } else {
      print ENTTAB " else";
   }
   my $len = length($_);
   print ENTTAB " if (strncmp(*p, \"$_\", $len) == 0 && !isalnum((*p)[$len]))\n";
   $ent{$_} =~ /(.)(.?)/;
   print ENTTAB "  ch = '$1',";
   print ENTTAB " cache = '$2'," if $2;
   print ENTTAB " *p += $len;\n";
}
print ENTTAB " break;\n";

undef $case;
$first = 0;
foreach (sort keys %entu) {
   my ($newcase) = /(.)/;
   if (!defined $case || $case ne $newcase) {
      print ENTTABU " break;\n" if defined $case;
      $case = $newcase;      
      print ENTTABU "case '$case':\n";
      $first = 1;
   }
   if ($first) {
      $first = 0;
   } else {
      print ENTTABU " else";
   }
   my $len = length($_);
   print ENTTABU " if (strcmp(p, \"$_\") == 0)\n  ch = $entu{$_};\n";
}
print ENTTABU " break;\n";
