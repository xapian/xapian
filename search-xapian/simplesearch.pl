#!/usr/bin/perl -w

use strict;

use ExtUtils::testlib;
use Getopt::Std;
use Search::Xapian;

use vars qw( %opts );
getopts('d:t:h', \%opts);

if( exists $opts{h} ) {
  print <<EOF;
Synopsis: Searches a Xapian database for a particular term.
Usage: $0 -d testdb -t help
Options:
        -d : database directory
        -t : search term
        -h : displays this help screen
EOF
  exit 0;
}

if( !$opts{d} or !$opts{t} ) {
  print "Usage: $0 -d [database dir] -t [search term]\n";
  print "Try $0 -h for further information\n";
  exit 0;
}

my $settings = Search::Xapian::Settings->new();

$settings->set( 'backend', 'auto' );
$settings->set( 'auto_dir', $opts{d} );

my $db = Search::Xapian::Database->new( $settings );
my $enq = Search::Xapian::Enquire->new( $db );
my $query = Search::Xapian::Query->new( $opts{t} );

printf "Parsing query '%s'\n", $query->get_description();

$enq->set_query( $query );

my $matches = $enq->get_mset( 0, 10 );

print $matches->get_matches_estimated() . " results found\n";

my $match = $matches->begin();
my $size = $matches->size();

while( $size-- ){
  printf "ID %d %d%% [ %s ]\n", $match->get_docid(), $match->get_percent(), $match->get_document()->get_data();
  $match->inc();
}
