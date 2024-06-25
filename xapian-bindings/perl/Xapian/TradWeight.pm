package Xapian::TradWeight;

=head1 NAME

Xapian::TradWeight - Traditional Probabilistic Weighting scheme.

=head1 DESCRIPTION

Traditional Probabilistic Weighting scheme, as described by the early papers
on Probabilistic Retrieval.  BM25 generally gives better results.

This class is deprecated - use C<BM25Weight(k, 0, 0, 1, 0)> instead (which it
is equivalent to).

=head1 METHODS

=over 4

=item new

Constructor. Either takes no parameters, or a single non-negative parameter k.
If k isn't specified, the default value used is 1.

=back

=head1 SEE ALSO

L<Xapian>,
L<Xapian::Enquire>

=cut
1;
