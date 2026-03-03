package Xapian::ValueIterator;

=head1 NAME

Xapian::ValueIterator - Iterate over values in a slot.

=head1 DESCRIPTION

This object represents a stream of document values. It overloads C<++> for
advancing the iterator, or you can explicitly call the C<inc> method.
This class also overloads C<eq>, C<ne>, C<==>, and C<!=>.

=head2 Compatibility with Search::Xapian

Search::Xapian overloads <""> (stringification) on this class to call
the C<get_value> method.  Call C<get_value> directly instead.

=head1 METHODS

=over 4

=item new

Constructor. Defaults to a uninitialized iterator.

=item clone

=item inc

Advance the iterator by one. (Called implicitly by C<++> overloading)

=item get_valueno

Return the number of the value slot at the current position.

=item get_value

Return the string in the value slot at current position.

=item get_description

Returns a string describing this object.

=item equal <valueiterator>

Checks if a valueiterator is the same as this valueiterator. Also overloaded as
the C<eq> and C<==> operators.

=item nequal <valueiterator>

Checks if a valueiterator is different from this valueiterator. Also overloaded
as the C<ne> and C<!=> operators.

=back

=head1 SEE ALSO

L<Xapian>,
L<Xapian::Document>

=cut
1;
