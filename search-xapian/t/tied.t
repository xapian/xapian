# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test;
use Devel::Peek;
BEGIN { plan tests => 22 };
use Search::Xapian qw(:ops);

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# None of the following tests can be expected to succeed without first
# creating a test database in the directory testdb.

my $db = Search::Xapian::Database->new( 'testdb' );
my $enq = Search::Xapian::Enquire->new( $db );
my $query = Search::Xapian::Query->new( 'test' );

$enq->set_query( $query );

my $mset;
ok( $mset = $enq->get_mset(0, 10) );
my @matches;
ok( @matches = $mset->items() );
my $match;
ok( $match = $matches[0] );
ok( $match->get_docid() );
ok( $match->get_percent() );

$matches[0] = 34;

my $doc;
ok( $doc = $match->get_document() );
ok( $doc->get_data() );

ok( exists $matches[1] );
ok( !exists $matches[10] );
ok( exists $matches[-1] );

# Test that "tying by hand" still works.
sub tie_mset {
    my @a;
    tie( @a, 'Search::Xapian::MSet::Tied', shift );
    return @a;
}
ok( $mset = $enq->get_mset(0, 1) );
ok( scalar(tie_mset($mset)) == 1 );

my @ematches;
ok( @ematches = $enq->matches(0, 2) );

ok( $match = $ematches[0] );
ok( $match->get_docid() );
ok( $match->get_percent() );

my $eset;
my $rset;

ok( $rset = Search::Xapian::RSet->new() );
$rset->add_document( 1 );

ok( $eset = $enq->get_eset( 10, $rset ) );
ok( $eset->size() != 0 );

my @eterms;
ok( @eterms = $eset->items() );
ok( scalar @eterms == $eset->size() );
ok( $eterms[0]->get_termname() eq $eset->begin()->get_termname() );

1;
