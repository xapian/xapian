package Search::Xapian::BoolWeight;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::Weight;

require DynaLoader;

our @ISA = qw( DynaLoader Search::Xapian::Weight);

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

1;

=head1 NAME

Search::Xapian::BoolWeight - Boolean Weighting scheme.

=head1 DESCRIPTION

Boolean Weighting scheme. All documents get a weight of 0.

=head1 METHODS

=over 4 

=item new

Constructor. Takes no arguments.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Enquire>

=cut

