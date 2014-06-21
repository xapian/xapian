# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More;
BEGIN { plan tests => 122 };
use Search::Xapian qw(:ops);

# FIXME: these tests pass in the XS version.
my $disable_fixme = 1;

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

# None of the following tests can be expected to succeed without first
# creating a test database in the directory testdb.

my $db;
ok( $db = Search::Xapian::Database->new( 'testdb' ), "test db opened ok" );

my $enq;
ok( $enq = $db->enquire(), "db enquirable" );

my @subqueries;
my $query;
ok( $subqueries[0] = Search::Xapian::Query->new( 'test' ), "one-term queries ok" );
is( $subqueries[0]->get_description, "Xapian::Query(test)", "query parsed correctly" );

# tests 5-14
foreach my $op (OP_OR, OP_AND, OP_NEAR, OP_PHRASE) {
  ok( $query = Search::Xapian::Query->new( $op, @subqueries ), "$Search::Xapian::OP_NAMES[$op] works with 1 object" );
  ok( $query = Search::Xapian::Query->new( $op, 'help' ), "$Search::Xapian::OP_NAMES[$op] works with 1 term" );
}
is( $query->get_description, "Xapian::Query(help)", "query parsed correctly" );

# tests 15-41
$subqueries[1] = Search::Xapian::Query->new( 'help' );
foreach my $op (OP_OR, OP_AND, OP_NEAR, OP_PHRASE,
		OP_AND_NOT, OP_XOR, OP_AND_MAYBE, OP_FILTER, OP_ELITE_SET) {
  ok( $query = Search::Xapian::Query->new( $op, @subqueries ), "$Search::Xapian::OP_NAMES[$op] works with 2 objects" );
  ok( $query = Search::Xapian::Query->new( $op, $subqueries[0], 'test'), "$Search::Xapian::OP_NAMES[$op] works with an object and a term" );
  ok( $query = Search::Xapian::Query->new( $op, 'test', 'help'), "$Search::Xapian::OP_NAMES[$op] works with 2 terms" );
}
is( $query->get_description, "Xapian::Query((test ELITE_SET 10 help))", "query parsed correctly" );

# tests 42-...
$subqueries[2] = Search::Xapian::Query->new( 'one' );
foreach my $op (OP_OR, OP_AND, OP_NEAR, OP_PHRASE ) {
  ok( $query = Search::Xapian::Query->new( $op, @subqueries ), "$Search::Xapian::OP_NAMES[$op] works with 3 objects" );
  ok( $query = Search::Xapian::Query->new( $op, 'test', 'help', 'one' ), "$Search::Xapian::OP_NAMES[$op] works with 3 terms" );
}
is( $query->get_description, "Xapian::Query((test PHRASE 3 help PHRASE 3 one))", "query parsed correctly" );

ok( $enq = $db->enquire( $query ), "db queries return ok"  );
ok( $enq = $db->enquire( OP_OR, 'test', 'help' ), "in-line db queries return ok" );
is( $db->get_spelling_suggestion( 'tost' ), 'test', "spelling suggestion ok" );

ok( $query = Search::Xapian::Query->new(OP_SCALE_WEIGHT, $query, 3.14), "OP_SCALE_WEIGHT understood" );

my $matches;
ok( $matches = $enq->get_mset( 0, 10 ), "match set returned ok" );
is( $matches->get_matches_estimated(), 2, "match set contains correct number of results" );
my $matches2;
ok( $matches2 = $enq->get_mset( 0, 1, 3 ), "match set with check_at_least returned ok" );
is( $matches2->get_matches_estimated(), 2, "match set contains correct number of results" );

my $match;
ok( $match = $matches->begin(), "match set iterator returned ok" );
is( $match, $matches->begin(), "match set returns consistent start point");
ok( $match++, "match set iterator can increment" );
isnt( $match, $matches->begin(), "match set iterator increments correctly" );
ok( $match->get_docid(), "document id returned ok" );
ok( $match->get_percent(), "percent relevance returned ok" );
is( $match->get_percent(), $matches->convert_to_percent($match->get_weight()),
	"converting a weight to a percentage works" );
is( $match->get_percent(), $matches->convert_to_percent($match),
	"converting an MSetIterator to a percentage works" );

my $doc;
ok( $doc = $match->get_document(), "documents retrievable" );
ok( $doc->get_data(), "data retrievable" );

ok( $match--, "match set iterator can decrement" );
is( $match, $matches->begin(), "match set iterator decrements correctly" );

for (1 .. $matches->size()) { $match++; }
is( $match, $matches->end(), "match set returns correct endpoint");

my $rset;
ok( $rset = Search::Xapian::RSet->new(), "relevance set created ok" );
$rset->add_document( 1 );
ok( $rset->contains( 1 ), "document added to relevance set successfully" );
ok( !$rset->contains( 2 ), "relevance set correctly fails to match document it does not contain" );
$rset->remove_document( 1 );
ok( !$rset->contains( 1 ), "document removed from relevance set successfully" );
$rset->add_document( 1 );

my $matches3;
ok( $matches3 = $enq->get_mset(0, 10, $rset), "get_mset with rset" );
is( $matches3->size, $matches->size, "rset doesn't change mset size" );
ok( $matches3 = $enq->get_mset(0, 10, 11, $rset), "get_mset with check_at_least and rset" );
is( $matches3->size, $matches->size, "rset and check_at_least don't change mset size" );

my $d;
# This was generating a warning converting "0" to an RSet object:
ok( $disable_fixme or
    $matches3 = $enq->get_mset(0, 10,
			sub { $d = scalar @_; return $_[0]->get_value(0) ne ""; }),
	"get_mset with matchdecider" );
ok( $disable_fixme || defined $d, "matchdecider was called" );
ok( $disable_fixme || $d == 1, "matchdecider got an argument" );

sub mdecider {
    $d = scalar @_;
    return $_[0]->get_value(0) ne "";
}

$d = undef;
ok( $disable_fixme or
    $matches3 = $enq->get_mset(0, 10, \&mdecider),
	"get_mset with named matchdecider function" );
ok( $disable_fixme || defined $d, "matchdecider was called" );
ok( $disable_fixme || $d == 1, "matchdecider got an argument" );

my $eset;
ok( $eset = $enq->get_eset( 10, $rset ), "can get expanded terms set" );
is( $eset->size(), 1, "expanded terms set of correct size" );
my $eit;
ok( $eit = $eset->begin(), "expanded terms set iterator retuns ok" );
is( $eit->get_termname(), 'one', "expanded terms set contains correct terms");
is( ++$eit, $eset->end(), "eset iterator reaches ESet::end() ok" );
--$eit;
is( $eit->get_termname(), 'one', "eset iterator decrement works ok" );
ok( $disable_fixme or $eset = $enq->get_eset( 10, $rset, sub { $_[0] ne "one" } ), "expanded terms set with decider" );
is( $disable_fixme ?0: $eset->size(), 0, "expanded terms decider filtered" );

# try an empty mset - this was giving begin != end
my ($noquery, $nomatches);
ok( $noquery = Search::Xapian::Query->new( OP_AND_NOT, 'test', 'test' ), "matchless query returns ok" );
$enq->set_query( $noquery );
ok( $nomatches = $enq->get_mset( 0, 10 ), "matchless query returns match set ok" );
is( $nomatches->size(), 0, "matchless query's match set has zero size" );
is( $nomatches->begin(), $nomatches->end(), "matchless query's match set's start point and endpoint are the same" );

ok( $matches->convert_to_percent(100) > 0 );
ok( $matches->convert_to_percent( $matches->begin() ) > 0 );

$match = $matches->back();
--$match;
++$match;
ok( $match eq $matches->back() );

ok( $match->get_collapse_count() == 0 );

my $bm25;
ok( $bm25 = Search::Xapian::BM25Weight->new() );

my $boolweight;
ok( $boolweight = Search::Xapian::BoolWeight->new() );

my $tradweight;
ok( $tradweight = Search::Xapian::TradWeight->new() );

my $alltermit = $db->allterms_begin();
ok( $alltermit != $db->allterms_end() );
ok( $disable_fixme || "$alltermit" eq 'one' );
ok( $alltermit->get_termname() eq 'one' );
ok( ++$alltermit != $db->allterms_end() );
ok( $disable_fixme || "$alltermit" eq 'test' );
ok( $alltermit->get_termname() eq 'test' );
ok( ++$alltermit != $db->allterms_end() );
ok( $disable_fixme || "$alltermit" eq 'two' );
ok( $alltermit->get_termname() eq 'two' );
ok( ++$alltermit == $db->allterms_end() );

$alltermit = $db->allterms_begin('t');
ok( $alltermit != $db->allterms_end('t') );
ok( $disable_fixme || "$alltermit" eq 'test' );
ok( $alltermit->get_termname() eq 'test' );
ok( ++$alltermit != $db->allterms_end('t') );
ok( $disable_fixme || "$alltermit" eq 'two' );
ok( $alltermit->get_termname() eq 'two' );
ok( ++$alltermit == $db->allterms_end('t') );

# Check that non-string scalars get coerced.
my $numberquery = Search::Xapian::Query->new( OP_OR, (12, "34", .5) );
is( $numberquery->get_description(), "Xapian::Query((12 OR 34 OR 0.5))" );

1;
