package Search::Xapian::SimpleStopper;

use 5.006;
use strict;
use warnings;

use Search::Xapian::Stopper;

require DynaLoader;

our @ISA = qw(DynaLoader Search::Xapian::Stopper);

# Preloaded methods go here.

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

#use overload '='  => sub { $_[0]->clone() },
#             'fallback' => 1;

sub new {
    my $class = shift;
    my $stopper = new0();

    bless $stopper, $class;
    foreach (@_) {
	$stopper->add($_);
    }

    return $stopper;
}

1;
