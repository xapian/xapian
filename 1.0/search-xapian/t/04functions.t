
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
BEGIN { plan tests => 14 };
use Search::Xapian qw(:standard);

ok(1); # If we made it this far, we're ok.

#########################

# Test non-class functions.

my $version = join(".", (
	Search::Xapian::major_version(),
	Search::Xapian::minor_version(),
	Search::Xapian::revision()
    ));

ok($version eq Search::Xapian::version_string());

for my $val (-9e10, -1234.56, -1, -1e-10, 0, 1e-10, 1, 2, 4, 8, 9, 9e10) {
    my $enc_val = Search::Xapian::sortable_serialise($val);
    ok(Search::Xapian::sortable_unserialise($enc_val) == $val);
}


1;
