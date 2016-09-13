use strict;
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
BEGIN { plan tests => 10 };
use Search::Xapian qw(:standard);
use Config;

ok(1); # If we made it this far, we're ok.

#########################

# Adjust query description from 1.4 to match.
sub qd {
    local $_ = (shift @_)->get_description();
    if (substr($_, 0, 1) eq 'Q') {
	s/\@([0-9]+)/:(pos=$1)/g;
	$_ = "Xapian::$_";
    }
    return $_;
}

my $query = Search::Xapian::Query->new(
	OP_AND,
	Search::Xapian::Query->new( "foo" ),
	"bar"
);
ok(defined $query, "AND query created");
is(qd($query), 'Xapian::Query((foo AND bar))', "AND query contains foo part");

$query = Search::Xapian::Query::MatchAll;
is(qd($query), 'Xapian::Query(<alldocuments>)');
$query = Search::Xapian::Query->new("");
is(qd($query), 'Xapian::Query(<alldocuments>)');
$query = Search::Xapian::Query::MatchNothing;
is(qd($query), 'Xapian::Query()');
$query = Search::Xapian::Query->new();
is(qd($query), 'Xapian::Query()');

eval {
    Search::Xapian::Query->new("hello", 1, 2, 3, 4);
};
ok(defined $@, "Bad query ctor threw exception");
like($@, qr!^USAGE: Search::Xapian::Query->new\('term'\) or Search::Xapian::Query->new\(OP, <args>\) at \S+/10query\.t line \d+\.?$!);

ok(1);
