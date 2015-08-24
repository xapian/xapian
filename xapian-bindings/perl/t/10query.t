
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
BEGIN { plan tests => 6 };
use Search::Xapian qw(:standard);
use Config;

ok(1); # If we made it this far, we're ok.

#########################

my $query = Search::Xapian::Query->new(
	OP_AND,
	Search::Xapian::Query->new( "foo" ),
	"bar"
);
ok(defined $query, "AND query created");
is($query->get_description, 'Xapian::Query((foo AND bar))', "AND query contains foo part");

$query = Search::Xapian::Query::MatchAll;
is($query->get_description, 'Xapian::Query(<alldocuments>)');
$query = Search::Xapian::Query::MatchNothing;
is($query->get_description, 'Xapian::Query()');

ok(1);
