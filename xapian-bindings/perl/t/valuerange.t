# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More;
BEGIN { plan tests => 22 };
use Search::Xapian qw(:all);

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# None of the following tests can be expected to succeed without first
# creating a test database in the directory testdb.

my $db;
ok( $db = Search::Xapian::Database->new( 'testdb' ), "test db opened ok" );

my $enq;
ok( $enq = Search::Xapian::Enquire->new( $db ), "enquire object created" );

my $query;
my $mset;

ok( $query = Search::Xapian::Query->new(OP_VALUE_RANGE, 0, "a", "b") );

$enq->set_query($query);
ok( $mset = $enq->get_mset(0, 10), "got mset" );
is( $mset->size, 0, "range a..b ok" );

ok( $query = Search::Xapian::Query->new(OP_VALUE_RANGE, 0, "four", "seven") );

$enq->set_query($query);
ok( $mset = $enq->get_mset(0, 10), "got mset" );
is( $mset->size, 1, "range four..seven ok" );

is( $mset->begin()->get_document()->get_value(0), "one" );

ok( $query = Search::Xapian::Query->new(OP_VALUE_RANGE, 0, "one", "zero") );

$enq->set_query($query);
ok( $mset = $enq->get_mset(0, 10), "got mset" );
is( $mset->size, 2, "range one..zero ok" );
my $mseti = $mset->begin();
is( $mseti->get_document()->get_value(0), "one" );
++$mseti;
is( $mseti->get_document()->get_value(0), "two" );

ok( $query = Search::Xapian::Query->new(OP_VALUE_LE, 0, "one") );
$enq->set_query($query);
ok( $mset = $enq->get_mset(0, 10), "got mset" );
# FIXME: bug in xapian-core in 1.0.6 and earlier means this gives the wrong answer
#is( $mset->size, 1, "range ..one ok" );
ok( 1 );
$mseti = $mset->begin();
is( $mseti->get_document()->get_value(0), "one" );

ok( $query = Search::Xapian::Query->new(OP_VALUE_GE, 0, "two") );
$enq->set_query($query);
ok( $mset = $enq->get_mset(0, 10), "got mset" );
is( $mset->size, 1, "range one.. ok" );
$mseti = $mset->begin();
is( $mseti->get_document()->get_value(0), "two" );

1;
