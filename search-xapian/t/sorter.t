# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More;
BEGIN { plan tests => 39 };
use Search::Xapian qw(:all);

#########################

# Insert your test code below, the Test module is use()ed here so read
# its man page ( perldoc Test ) for help writing this test script.

sub mset_expect_order (\@@) {
    my ($m, @a) = @_;
    my @m = map { $_->get_docid() } @{$m};
    is( scalar @m, scalar @a );
    for my $j (0 .. (scalar @a - 1)) {
	is( $m[$j], $a[$j] );
    }
}

my $db;
ok( $db = Search::Xapian::WritableDatabase->new(), "test db opened ok" );

my $enquire;
ok( $enquire = Search::Xapian::Enquire->new( $db ), "enquire object created" );

my $doc;
ok( $doc = Search::Xapian::Document->new() );
$doc->add_term("foo");
$doc->add_value(0, "ABB");
$db->add_document($doc);
$doc->add_value(0, "ABC");
$db->add_document($doc);
$doc->add_value(0, "ABC\0");
$db->add_document($doc);
$doc->add_value(0, "ABCD");
$db->add_document($doc);
$doc->add_value(0, "ABC\xff");
$db->add_document($doc);

$enquire->set_query(Search::Xapian::Query->new("foo"));

{
    {
	my $sorter = Search::Xapian::MultiValueSorter->new();
	$sorter->add(0);
	$enquire->set_sort_by_key($sorter);
    }
    my @matches = $enquire->matches(0, 10);
    mset_expect_order(@matches, (5, 4, 3, 2, 1));
}

{
    my $sorter = Search::Xapian::MultiValueSorter->new();
    $sorter->add(0, 0);
    $enquire->set_sort_by_key($sorter);
    my @matches = $enquire->matches(0, 10);
    mset_expect_order(@matches, (1, 2, 3, 4, 5));
}

{
    my $sorter = Search::Xapian::MultiValueSorter->new();
    $sorter->add(0);
    $sorter->add(1);
    $enquire->set_sort_by_key($sorter);
    my @matches = $enquire->matches(0, 10);
    mset_expect_order(@matches, (5, 4, 3, 2, 1));
}

{
    my $sorter = Search::Xapian::MultiValueSorter->new();
    $sorter->add(0, 0);
    $sorter->add(1);
    $enquire->set_sort_by_key($sorter);
    my @matches = $enquire->matches(0, 10);
    mset_expect_order(@matches, (1, 2, 3, 4, 5));
}

{
    my $sorter = Search::Xapian::MultiValueSorter->new();
    $sorter->add(0);
    $sorter->add(1, 0);
    $enquire->set_sort_by_key($sorter);
    my @matches = $enquire->matches(0, 10);
    mset_expect_order(@matches, (5, 4, 3, 2, 1));
}

{
    my $sorter = Search::Xapian::MultiValueSorter->new();
    $sorter->add(0, 0);
    $sorter->add(1, 0);
    $enquire->set_sort_by_key($sorter);
    my @matches = $enquire->matches(0, 10);
    mset_expect_order(@matches, (1, 2, 3, 4, 5));
}

1;
