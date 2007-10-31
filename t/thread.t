#!/usr/bin/perl -W
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
if ($] < 5.008007) {
    plan skip_all => 'Test requires Perl >= 5.8.7';
} else {
    # Number of test cases to run - increase this if you add more testcases.
    plan tests => 29;
}

use Search::Xapian qw(:standard);
use threads;

my ($wdb, $db, $doc, $bm25wt, $boolwt, $tradwt, $enq, $qp, $q);

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
    return 0 unless $@ && ref($bm25wt) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless $@ && ref($boolwt) !~ 'Xapian';
    # Check that it isn't a Xapian object.
    return 0 unless $@ && ref($tradwt) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $enq->get_query(); };
    return 0 unless $@ && ref($enq) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $qp->get_default_op(); };
    return 0 unless $@ && ref($qp) !~ 'Xapian';
    # Check that calling a method fails, and that it isn't a Xapian object.
    eval { $q->empty(); };
    return 0 unless $@ && ref($q) !~ 'Xapian';
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
