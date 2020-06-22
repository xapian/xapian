package Xapian::PostingIterator;

=head1 NAME

Xapian::PostingIterator - Iterate over the list of documents indexed
by a term.

=head1 DESCRIPTION

This iterator represents a stream of documents indexed by a term. It overloads
C<++> for advancing the iterator, or you can explicitly call the C<inc> method.
This class also overloads C<eq>, C<ne>, C<==>, and C<!=>.

=head2 Compatibility with Search::Xapian

Search::Xapian overloads <""> (stringification) on this class to call
the C<get_description> method.  Call C<get_description> explicitly instead.

Search::Xapian overloads <0+> (convert to an integer) on this class to call
the C<get_docid> method.  Call C<get_docid> explicitly instead.

=head1 METHODS

=over 4

=item new

Constructor. Defaults to an uninitialized iterator.

=item clone

=item inc

Advance the iterator by one. (Called implicitly by C<++> overloading).

=item skip_to <docid>

Advance the iterator to document docid, or the first document after docid if docid
isn't in the list of documents being iterated.

=item get_docid

Get the unique id of the document at the current position of the iterator.

=item get_wdf

Return the wdf for the current position of the iterator.

=item positionlist_begin

Return L<Xapian::PositionIterator> pointing to start of positionlist for
current document.

=item positionlist_end

Return L<Xapian::PositionIterator> pointing to end of positionlist for
current document.

=item get_doclength

Get the length of the document at the current position of the iterator.

=item equal <postingiterator>

Compare for equality with another Xapian::PostingIterator object. Also
overloaded to the C<eq> and C<==> operators.

=item nequal <postingiterator>

Compare for inequality with another Xapian::PostingIterator object. Also
overloaded to the C<ne> and C<!=> operators.

=item get_description

Return a description of this object.

=back

=head1 SEE ALSO

L<Xapian>,
L<Xapian::Database>

=cut
1;
