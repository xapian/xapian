package Search::Xapian::Weight;

use 5.006;
use strict;
use warnings;

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

1;

__END__

=head1 NAME

Search::Xapian::Weight - base class for Weighting schemes.

=head1 DESCRIPTION

This is an abstract base class for weighting schemes in Xapian.

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::BoolWeight>,L<Search::Xapian::BM25Weight>

=cut
