
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use Test::More;
BEGIN { plan tests => 26 };
use Search::Xapian qw(:standard);
use Config;

ok(1); # If we made it this far, we're ok.

#########################

# Test non-class functions.

my $version = join(".", (
	Search::Xapian::major_version(),
	Search::Xapian::minor_version(),
	Search::Xapian::revision()
    ));

ok($version eq Search::Xapian::version_string());

my $ldbl = (defined($Config{uselongdouble}) &&
	    $Config{uselongdouble} eq "define" &&
	    $Config{doublesize} != $Config{longdblsize});
for my $val (-9e10, -1234.56, -1, -1e-10, 0, 1e-10, 1, 2, 4, 8, 9, 9e10) {
    my $enc_val = Search::Xapian::sortable_serialise($val);
    if ($ldbl) {
	# Perl is configured with -Duselongdouble and long double isn't the
	# same as double, so we may have rounded the input value slightly
	# passing it to Xapian.
	my $de_enc_val = Search::Xapian::sortable_unserialise($enc_val);
	ok(abs($de_enc_val - $val) < 1e-12);
	# But encoding and decoding the value we got back should give exactly
	# the same value.
	my $re_enc_val = Search::Xapian::sortable_serialise($de_enc_val);
	ok(Search::Xapian::sortable_unserialise($re_enc_val) == $de_enc_val);
    } else {
	# We should get the exact same value back.
	ok(Search::Xapian::sortable_unserialise($enc_val) == $val);
	ok(1);
    }
}


1;
