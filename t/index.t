# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 38 };
use Search::Xapian qw(:standard);

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

foreach my $backend ("inmemory", "auto") {
  my $database;
  if ($backend eq "inmemory") {
    ok( $database = Search::Xapian::WritableDatabase->new() );
  } else {
    ok( $database = Search::Xapian::WritableDatabase->new( 'testdb', Search::Xapian::DB_CREATE_OR_OVERWRITE ) );
  }

  ok( $database->get_description() );

  my $stemmer;
  ok( $stemmer = Search::Xapian::Stem->new( 'english' ) );
  ok( $stemmer->get_description() );

  my %docs;

  my $term = 'test';
  ok( $term = $stemmer->stem_word( $term ) );

  my $docid;
  for my $num qw( one two three ) {
    ok( $docs{$num} = Search::Xapian::Document->new() );
    ok( $docs{$num}->get_description() );

    $docs{$num}->set_data( "$term $num" );

    $docs{$num}->add_posting( $term, 0 );
    $docs{$num}->add_posting( $num, 1 );

    ok( $docid = $database->add_document( $docs{$num} ) );
  }
  $database->delete_document( $docid );
  ok( $database->get_doccount(), 2 );
  ok( $database->get_lastdocid(), 3 );

  my $posit = $database->positionlist_begin(1, $term);
  ok( $posit ne $database->positionlist_end(1, $term) );
  ok( $posit == 0 );
  $posit++;
  ok( $posit eq $database->positionlist_end(1, $term) );
}

1;
