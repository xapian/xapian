# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
if ($] < 5.008007) {
    # Perl 5.8.7 added CLONE_SKIP which is required to implement the behaviour
    # which this test case tests.
    plan skip_all => 'Test requires Perl >= 5.8.7 for CLONE_SKIP';
}
eval {
    require threads;
};
if ($@) {
    plan skip_all => 'Test requires Perl with thread support';
}

# Number of test cases to run - increase this if you add more testcases.
plan tests => 65;

use Search::Xapian qw(:standard);

# TODO: check these classes too:
# MSet/Tied.pm
# PerlStopper.pm
# Stopper.pm
# Weight.pm

my ($wdb, $db, $doc, $bm25wt, $boolwt, $tradwt, $enq, $qp, $q, $stem);
my ($eset, $mset, $rset, $esetit, $msetit, $postit, $posit, $termit, $valueit);
my ($sstop, $tg);

sub thread_proc {
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $wdb->get_doccount(); };
    return 0 unless $@ && ref($wdb) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $db->get_doccount(); };
    return 0 unless $@ && ref($db) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $doc->get_data(); };
    return 0 unless $@ && ref($doc) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($bm25wt) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($boolwt) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($tradwt) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $enq->get_query(); };
    return 0 unless $@ && ref($enq) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $qp->get_default_op(); };
    return 0 unless $@ && ref($qp) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $q->empty(); };
    return 0 unless $@ && ref($q) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $stem->stem_word("testing"); };
    return 0 unless $@ && ref($stem) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $eset->empty(); };
    return 0 unless $@ && ref($eset) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $mset->empty(); };
    return 0 unless $@ && ref($mset) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $rset->empty(); };
    return 0 unless $@ && ref($rset) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($esetit) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($msetit) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($postit) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($posit) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($termit) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($valueit) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($sstop) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless ref($tg) !~ 'Xapian';
}

ok( $wdb = Search::Xapian::WritableDatabase->new(), 'create WritableDatabase' );
is( $wdb->get_doccount(), 0, 'check WritableDatabase' );

ok( $db = Search::Xapian::Database->new('testdb'), 'create Database' );
is( $db->get_doccount(), 2, 'check Database' );

ok( $doc = $db->get_document(1), 'create Document' );
is( $doc->get_data(), 'test one', 'check Document' );

ok( $bm25wt = Search::Xapian::BM25Weight->new(), 'create BM25Weight' );
is( ref($bm25wt), 'Search::Xapian::BM25Weight', 'check BM25Weight' );

ok( $boolwt = Search::Xapian::BoolWeight->new(), 'create BoolWeight' );
is( ref($boolwt), 'Search::Xapian::BoolWeight', 'check BoolWeight' );

ok( $tradwt = Search::Xapian::TradWeight->new(), 'create TradWeight' );
is( ref($tradwt), 'Search::Xapian::TradWeight', 'check TradWeight' );

ok( $enq = Search::Xapian::Enquire->new($db), 'create Enquire' );
ok( $enq->get_query()->empty(), 'check Enquire' );

ok( $qp = Search::Xapian::QueryParser->new(), 'create QueryParser' );
is( $qp->get_default_op(), OP_OR, 'check QueryParser' );

ok( $q = $qp->parse_query("foo"), 'create Query' );
ok( !$q->empty(), 'check Query' );

ok( $stem = Search::Xapian::Stem->new('en'), 'create Stem' );
is( $stem->stem_word('testing'), 'test', 'check Stem' );

ok( $eset = Search::Xapian::ESet->new(), 'create ESet' );
ok( $eset->empty(), 'check ESet' );

ok( $mset = Search::Xapian::MSet->new(), 'create MSet' );
ok( $mset->empty(), 'check MSet' );

ok( $rset = Search::Xapian::RSet->new(), 'create RSet' );
ok( $rset->empty(), 'check RSet' );

ok( $esetit = $eset->begin(), 'create ESetIterator' );
is( ref($esetit), 'Search::Xapian::ESetIterator', 'check ESetIterator' );

ok( $msetit = $mset->begin(), 'create MSetIterator' );
is( ref($msetit), 'Search::Xapian::MSetIterator', 'check MSetIterator' );

ok( $postit = $db->postlist_begin("one"), 'create PostingIterator' );
is( ref($postit), 'Search::Xapian::PostingIterator', 'check PostingIterator' );

ok( $posit = $db->positionlist_begin(1, "one"), 'create PositionIterator' );
is( ref($posit), 'Search::Xapian::PositionIterator', 'check PositionIterator' );

ok( $termit = $db->termlist_begin(1), 'create TermIterator' );
is( ref($termit), 'Search::Xapian::TermIterator', 'check TermIterator' );

ok( $valueit = $doc->values_begin(), 'create ValueIterator' );
is( $valueit->get_valueno(), 0, 'check ValueIterator' );

ok( $sstop = Search::Xapian::SimpleStopper->new(), 'create SimpleStopper' );
is( ref($sstop), 'Search::Xapian::SimpleStopper', 'check SimpleStopper' );

ok( $tg = Search::Xapian::TermGenerator->new(), 'create TermGenerator' );
is( ref($tg), 'Search::Xapian::TermGenerator', 'check TermGenerator' );

my $thread1 = threads->create(sub { thread_proc(); });
my $thread2 = threads->create(sub { thread_proc(); });
ok( $thread1->join, 'check thread1' );
ok( $thread2->join, 'check thread2' );

is( $wdb->get_doccount(), 0, 'check WritableDatabase' );
is( $db->get_doccount(), 2, 'check Database' );
is( $doc->get_data(), 'test one', 'check Document' );
is( ref($bm25wt), 'Search::Xapian::BM25Weight', 'check BM25Weight' );
is( ref($boolwt), 'Search::Xapian::BoolWeight', 'check BoolWeight' );
is( ref($tradwt), 'Search::Xapian::TradWeight', 'check TradWeight' );
ok( $enq->get_query()->empty(), 'check Enquire' );
is( $qp->get_default_op(), OP_OR, 'check QueryParser' );
ok( !$q->empty(), 'check Query' );
is( $stem->stem_word('testing'), 'test', 'check Stem' );
ok( $eset->empty(), 'check ESet' );
ok( $mset->empty(), 'check MSet' );
ok( $rset->empty(), 'check RSet' );
is( ref($esetit), 'Search::Xapian::ESetIterator', 'check ESetIterator' );
is( ref($msetit), 'Search::Xapian::MSetIterator', 'check MSetIterator' );
is( ref($postit), 'Search::Xapian::PostingIterator', 'check PostingIterator' );
is( ref($posit), 'Search::Xapian::PositionIterator', 'check PositionIterator' );
is( ref($termit), 'Search::Xapian::TermIterator', 'check TermIterator' );
is( $valueit->get_valueno(), 0, 'check ValueIterator' );
is( ref($sstop), 'Search::Xapian::SimpleStopper', 'check SimpleStopper' );
is( ref($tg), 'Search::Xapian::TermGenerator', 'check TermGenerator' );
