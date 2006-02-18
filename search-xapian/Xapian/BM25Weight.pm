package Search::Xapian::BM25Weight;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::Weight;

require DynaLoader;

our @ISA = qw( DynaLoader Search::Xapian::Weight);

1;

=head1 NAME

Search::Xapian::BM25Weight - BM25 Weighting scheme.

=head1 DESCRIPTION

BM25 Weighting scheme. This is the default weighting scheme.

=head1 METHODS

=over 4 

=item new

Constructor. Takes no arguments.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Enquire>

=cut

