package Xapian::Query;

=head1 NAME

Xapian::Query - class representing a query.

=head1 DESCRIPTION

=head2 Compatibility with Search::Xapian

Search::Xapian overloads <""> (stringification) on this class to call
the C<get_description> method.  Call C<get_description> directly instead.

=head1 METHODS

=over 4

=item get_description

Return a description of this object.

=back

=head1 SEE ALSO

L<Xapian>,
L<Xapian::Enquire>

=cut
1;
