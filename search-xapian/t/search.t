# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 13 };
use Search::Xapian;

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

my $settings;
$settings = Search::Xapian::Settings->new();

$settings->set( 'backend', 'auto' );
$settings->set( 'auto_dir', 'testdb' );

# None of the following tests can be expected to succeed without first
# creating a test database in the directory testdb.
# Feel free to remove the above 'exit' line, create your own database
# using the xapian C++ examples and use the following tests on this module

my $db;
ok( $db = Search::Xapian::Database->new( $settings ) );

my $enq;
ok( $enq = Search::Xapian::Enquire->new( $db ) );

my $query;
ok( $query = Search::Xapian::Query->new( 'test' ) );
ok( $query->get_description() );

$enq->set_query( $query );

my $matches;
ok( $matches = $enq->get_mset( 0, 10 ) );
ok( $matches->get_matches_estimated() );
ok( $matches->size() );

my $match;
ok( $match = $matches->begin() );
ok( $match->inc() );
ok( $match->get_docid() );
ok( $match->get_percent() );

my $doc;
ok( $doc = $match->get_document() );
ok( $doc->get_data() );

1;
