
# Before 'make install' is performed this script should be runnable with
# 'make test'. After 'make install' it should work as 'perl test.pl'

#########################

# Make warnings fatal
use warnings;
BEGIN {$SIG{__WARN__} = sub { die "Terminating test due to warning: $_[0]" } };

use Test::More;
BEGIN { plan tests => 2 };
use Xapian qw(:standard);

ok(1); # If we made it this far, we're ok.

#########################

foreach my $l (split(/ /, Xapian::Stem::get_available_languages())) {
    my $s = new Xapian::Stem($l);
}
ok(1);
