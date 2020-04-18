use strict;
# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

#########################

# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test::More;
BEGIN { plan tests => 5 };
use Xapian qw(:standard);

ok(1); # If we made it this far, we're ok.

#########################

foreach my $l (split(/ /, Xapian::Stem::get_available_languages())) {
    my $s = new Xapian::Stem($l);
}

# Test user-specified stemmers, using the naive "truncate to 3 characters"
# stemmer.
my $stemmer;
ok( $stemmer = new Xapian::Stem(sub {substr($_[0], 0, 3)}) );
is( $stemmer->stem_word("honey"), "hon" );
is( $stemmer->stem_word("b"), "b" );

ok(1);
