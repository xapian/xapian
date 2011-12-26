package Search::Xapian::PositionIterator;

=head1 NAME

Search::Xapian::PositionIterator - Iterate over sets of positions.

=head1 DESCRIPTION

This iterator represents a stream of positions for a term. It overloads
C<++> for advancing the iterator, or you can explicitly call the C<inc> method.
This class also overloads C<eq>, C<ne>, C<==>, C<!=>, C<"">
(stringification) and C<0+> (conversion to an integer).

=head1 METHODS

=over 4

=item new

Constructor. Defaults to an uninitialized iterator.

=item clone

=item inc

Advance the iterator by one. (Called implictly by C<++> overloading).

=item skip_to <termpos>

Skip the iterator to term position termpos, or the first term position after
termpos if termpos isn't in the list of term positions being iterated.

=item equal <term>

Checks if a term is the same as this term. Also overloaded to the C<eq>
and C<==> operators.

=item nequal <term>

Checks if a term is different from this term. Also overloaded to the C<ne>
and C<!=> operators.

=item get_termpos

Return the term position the iterator is currently on. Also implemented as
conversion to an integer.

=item get_description

Return a description of this object.  Also implemented as stringification.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Document>

=cut
1;
