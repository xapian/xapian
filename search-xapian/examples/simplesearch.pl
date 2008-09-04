#!/usr/bin/perl -w

use strict;

use ExtUtils::testlib;
use Getopt::Std;
use Search::Xapian qw(:ops);

use vars qw( %opts );
getopts('d:t:nh', \%opts);

if( exists $opts{h} ) {
  print <<EOF;
Synopsis: Searches a Xapian database for a particular term.
Usage: $0 -d testdb -t help
Options:
        -d : database directory
        -t : search terms
        -h : displays this help screen
        -q : quiet; does not display document data
EOF
  exit 0;
}

if( !$opts{d} or !$opts{t} ) {
  print "Usage: $0 -d [database dir] -t [search terms]\n";
  print "Try $0 -h for further information\n";
  exit 0;
}

my $db = Search::Xapian::Database->new( $opts{d} );
my @terms = split ',', $opts{t};
my $enq = $db->enquire( OP_OR, @terms );

printf "Parsing query '%s'\n", $enq->get_query()->get_description();

my @matches = $enq->matches(0, 10);

print scalar(@matches) . " results found\n";

foreach my $match ( @matches ) {
  printf "ID %d %d%%", $match->get_docid(), $match->get_percent();
  if( !defined( $opts{n} ) ) {
    my $doc = $match->get_document();
    printf " [ %s ]", $doc->get_data();
  }
  print "\n";
}
