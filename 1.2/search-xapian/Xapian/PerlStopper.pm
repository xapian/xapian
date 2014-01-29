package Search::Xapian::PerlStopper;

use 5.006;
use strict;
use warnings;

use Search::Xapian::Stopper;

@Search::Xapian::PerlStopper::ISA = qw(Search::Xapian::Stopper);

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

1;
