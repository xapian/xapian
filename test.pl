# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 2 };
use Search::Xapian;
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

my $settings;
ok( $settings = Search::Xapian::Settings->new() );

$settings->set( 'backend', 'auto' );
$settings->set( 'auto_dir', 'testdb' );

exit 0;

# None of the following tests can be expected to succeed without first
# creating a test database in the directory testdb.
# Feel free to remove the above 'exit' line, create your own database
# using the xapian C++ examples and use the following tests on this module

my $db;
todo( $db = Search::Xapian::Database->new( $settings ) );

my $enq;
todo( $enq = Search::Xapian::Enquire->new( $db ) );

my $query;
todo( $query = Search::Xapian::Query->new( 'help' ) );
todo( $query->get_description() );

$enq->set_query( $query );

my $matches;
todo( $matches = $enq->get_mset( 0, 10 ) );
todo( $matches->get_matches_estimated() );
todo( $matches->size() );

my $match;
todo( $match = $matches->begin() );
todo( $match->inc() );
todo( $match->get_docid() );
todo( $match->get_percent() );

my $doc;
todo( $doc = $match->get_document() );
todo( $doc->get_data() );

1;
