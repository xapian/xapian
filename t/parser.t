# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More;
use Devel::Peek;
BEGIN { plan tests => 4 };
use Search::Xapian qw(:standard);
ok(1); # If we made it this far, we're ok.

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# first create database dir, if it doesn't exist;
my $db_dir = 'testdb';

my $database;
ok( $database = Search::Xapian::Database->new( $db_dir ) );

my $qp = new Search::Xapian::QueryParser( $database );
$qp = new Search::Xapian::QueryParser();

$qp->set_stemming_options( 'english', 1 );
$qp->set_default_op( OP_AND );

my $query;
ok( $query = $qp->parse_query( 'one OR (two AND three)' ) );

ok( my $enq = $database->enquire( $query ) );

1;
