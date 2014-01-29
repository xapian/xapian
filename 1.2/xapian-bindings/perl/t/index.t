# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More;
BEGIN { plan tests => 95 };
use Search::Xapian qw(:standard);

# FIXME: these tests pass in the XS version.
my $disable_fixme = 1;

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
  if ($backend ne "inmemory") {
    # inmemory doesn't implement spelling correction support.
    $database->add_spelling( $term, 1 );
  }

  my $docid;
  for my $num (qw( one two three )) {
    ok( $docs{$num} = Search::Xapian::Document->new() );
    ok( $docs{$num}->get_description() );

    $docs{$num}->set_data( "$term $num" );

    $docs{$num}->add_posting( $term, 0 );
    $docs{$num}->add_posting( $num, 1 );

    $docs{$num}->add_value(0, $num);

    if ($backend ne "inmemory") {
      # inmemory doesn't implement spelling correction support.
      $database->add_spelling( "x" . $term, 1 );
      $database->add_spelling( $term, 1 );
      $database->remove_spelling( "x" . $term, 1 );
    }
    ok( $docid = $database->add_document( $docs{$num} ) );
  }
  $database->delete_document( $docid );
  is( $database->get_doccount(), 2 );
  is( $database->get_lastdocid(), 3 );

  is( $database->get_document(1)->get_docid(), 1 );
  is( $database->get_document(2)->get_docid(), 2 );

  # regression test - add_posting with 2 parameters set wdfinc 0 in <=0.8.3.0
  ok( $database->get_doclength(1) == 2 );

  is( $database->get_document(1)->get_value(0), "one" );
  is( $database->get_document(2)->get_value(0), "two" );

  my $posit = $database->positionlist_begin(1, $term);
  ok( $posit ne $database->positionlist_end(1, $term) );
  ok( $posit == 0 );
  $posit++;
  ok( $posit eq $database->positionlist_end(1, $term) );

  my $alltermit = $database->allterms_begin();
  ok( $alltermit != $database->allterms_end() );
  ok( $disable_fixme || "$alltermit" eq 'one' );
  ok( $alltermit->get_termname() eq 'one' );
  ok( ++$alltermit != $database->allterms_end() );
  ok( $disable_fixme || "$alltermit" eq 'test' );
  ok( $alltermit->get_termname() eq 'test' );
  ok( ++$alltermit != $database->allterms_end() );
  ok( $disable_fixme || "$alltermit" eq 'two' );
  ok( $alltermit->get_termname() eq 'two' );
  ok( ++$alltermit == $database->allterms_end() );

  $alltermit = $database->allterms_begin('t');
  ok( $alltermit != $database->allterms_end('t') );
  ok( $disable_fixme || "$alltermit" eq 'test' );
  ok( $alltermit->get_termname() eq 'test' );
  ok( ++$alltermit != $database->allterms_end('t') );
  ok( $disable_fixme || "$alltermit" eq 'two' );
  ok( $alltermit->get_termname() eq 'two' );
  ok( ++$alltermit == $database->allterms_end('t') );

  # Feature test for metadata support.
  is( $database->get_metadata( "nothing" ), "" );
  is( $database->get_metadata( "foo" ), "" );
  $database->set_metadata( "foo", "bar" );
  is( $database->get_metadata( "nothing" ), "" );
  is( $database->get_metadata( "foo" ), "bar" );
}

# Check that trying to create an invalid stemmer gives an exception, not an
# abort.
eval {
  my $badstem = Search::Xapian::Stem->new( 'gibberish' );
};
ok($@);
ok(ref($@), "Search::Xapian::InvalidArgumentError");
ok($@->isa('Search::Xapian::Error'));
ok($@->get_msg, "Language code gibberish unknown");
ok( $disable_fixme || "$@" =~ /^Exception: Language code gibberish unknown(?: at \S+ line \d+\.)?$/ );

1;
