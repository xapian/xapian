#!/usr/bin/perl -W
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
if ($] < 5.008007) {
    plan skip_all => 'Test requires Perl >= 5.8.7';
} else {
    # Number of test cases to run - increase this if you add more testcases.
    plan tests => 5;
}

use Search::Xapian qw(:standard);
use threads;

my $db;

sub thread_proc {
    eval { $db->get_doccount(); };
    # Check that calling methods of $db fails, and $db isn't a Xapian class.
    return $@ && ref($db) !~ 'Xapian';
}

ok( $db = Search::Xapian::WritableDatabase->new(), 'create object' );
is( $db->get_doccount(), 0, 'check object' );
my $thread1 = threads->create(sub { thread_proc(); });
my $thread2 = threads->create(sub { thread_proc(); });
ok( $thread1->join, 'check thread1' );
ok( $thread2->join, 'check thread2' );
is( $db->get_doccount(), 0, 'check object' );
