# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test;
use Devel::Peek;
BEGIN { plan tests => 32 };
use Search::Xapian qw(:ops);

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# None of the following tests can be expected to succeed without first
# creating a test database in the directory testdb.

my $db;
ok( $db = Search::Xapian::Database->new( 'testdb' ) );

my $enq;
ok( $enq = $db->enquire() );

my ($query1, $query2, $query3, $query4);
ok( $query1 = Search::Xapian::Query->new( 'test' ) );
ok( $query2 = Search::Xapian::Query->new( OP_OR, 'test', 'help' ) );
ok( $query3 = Search::Xapian::Query->new( OP_OR, $query1, $query2 ) );
ok( $query4 = Search::Xapian::Query->new( OP_OR, 'test', 'help', 'one', 'two', 'three' ) );
ok( $query4->get_description() );

ok( $enq = $db->enquire( $query2 ) );
ok( $enq = $db->enquire( OP_OR, 'test', 'help' ) );

my $matches;
ok( $matches = $enq->get_mset( 0, 10 ) );
ok( $matches->get_matches_estimated() );
ok( $matches->size() );

my $match;
ok( $match = $matches->begin() );
ok( $match eq $matches->begin() );
ok( $match++ );
ok( $match ne $matches->end() );
ok( $match->get_docid() );
ok( $match->get_percent() );

my $doc;
ok( $doc = $match->get_document() );
ok( $doc->get_data() );

for (2 .. $matches->size()) { $match++; }
ok( $match eq $matches->end() );

my $rset;
ok( $rset = Search::Xapian::RSet->new() );
$rset->add_document( 1 );
ok( $rset->contains( 1 ) );
ok( !$rset->contains( 2 ) );

my $eset;
ok( $eset = $enq->get_eset( 10, $rset ) );
ok( $eset->size() );
my $eit;
ok( $eit = $eset->begin() );
ok( $eit->get_termname() );

# try an empty mset - this was giving begin != end
my ($noquery, $nomatches);
ok( $noquery = Search::Xapian::Query->new( OP_AND_NOT, 'test', 'test' ) );
$enq->set_query( $noquery );
ok( $nomatches = $enq->get_mset( 0, 10 ) );
ok( $nomatches->size() == 0 );
ok( $nomatches->begin() eq $nomatches->end() );

1;
