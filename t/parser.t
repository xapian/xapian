# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test;
use Devel::Peek;
BEGIN { plan tests => 16 };
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

$qp->set_stemmer( Search::Xapian::Stem->new('english') );
$qp->set_stemming_strategy( STEM_ALL );
$qp->set_default_op( OP_AND );

my $query;
ok( $query = $qp->parse_query( 'one OR (two AND three)' ) );

ok( my $enq = $database->enquire( $query ) );

my @stopwords = qw(a the in on and);
my $stopper;
ok( $stopper = new Search::Xapian::SimpleStopper(@stopwords) );
foreach (@stopwords) {
    ok( $stopper->stop_word($_) );
}
foreach (qw(one two three four five)) {
    ok( !$stopper->stop_word($_) );
}
ok( $qp->set_stopper($stopper), undef );

1;
