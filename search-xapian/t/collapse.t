# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test::More;
BEGIN { plan tests => 8 };
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
$doc->add_value(0, "A");
$doc->add_term("foo");
$db->add_document($doc);
$doc->add_term("foo");
$db->add_document($doc);
$doc->add_term("foo");
$db->add_document($doc);
$doc->add_term("foo");
$db->add_document($doc);
$doc->add_term("foo");
$db->add_document($doc);

$enquire->set_query(Search::Xapian::Query->new("foo"));

{
    $enquire->set_collapse_key(0);
    my @matches = $enquire->matches(0, 3);
    mset_expect_order(@matches, (5));
}

{
    $enquire->set_collapse_key(0, 2);
    my @matches = $enquire->matches(0, 3);
    mset_expect_order(@matches, (5, 4));
}

1;
