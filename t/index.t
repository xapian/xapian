# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 8 };
use Search::Xapian;

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

my $settings;
ok( $settings = Search::Xapian::Settings->new() );

$settings->set( 'backend', 'auto' );
$settings->set( 'auto_dir', 'testdb' );

my $database;
ok( $database = Search::Xapian::WritableDatabase->new( $settings ) );

my $stemmer;
ok( $stemmer = Search::Xapian::Stem->new( 'english' ) );

my %docs;

my $term = 'test';
ok( $term = $stemmer->stem_word( $term ) );

for my $num qw( one two ) {
  ok( $doc{$num} = Search::Xapian::Document->new() );

  $doc{$num}->set_data( "$term $num" );

  $doc{$num}->add_posting( $term, 0 );
  $doc{$num}->add_posting( $num, 1 );

  ok( $database->add_document( $doc{$num} ) );
}

1;