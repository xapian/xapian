package Search::Xapian::MatchSpy;

use 5.006;
use strict;
use warnings;

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

1;

__END__

=head1 NAME

Search::Xapian::MatchSpy - abstract base class for match spies.

=head1 DESCRIPTION

This is an abstract base class for match spies in Xapian.

The subclasses will generally accumulate information seen during the match, to calculate aggregate functions, or other profiles of the matching documents.

=head1 METHODS

=over 4

=item operator() <Document> <weight>

Virtual - needs to be implemented in extending classes. Registers a document with the match spy.

This is called by the matcher once with each document seen by the matcher during the match process. Note that the matcher will often not see all the documents which match the query, due to optimisations which allow low-weighted documents to be skipped, and allow the match process to be terminated early.

=item get_description

Return a string describing this object.

This default implementation returns a generic answer, to avoid forcing those deriving their own MatchSpy subclasses from having to implement this (they may not care what get_description() gives for their subclass).

Reimplemented in Xapian::ValueCountMatchSpy.

=item name

Return the name of this match spy.

This name is used by the remote backend. It is passed with the serialised parameters to the remote server so that it knows which class to create. Return the full namespace-qualified name of your class here. If you don't want to support the remote backend in your match spy, you can use the default implementation which simply throws Xapian::UnimplementedError.

Reimplemented in Xapian::ValueCountMatchSpy.


=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::ValueCountMatchSpy>,L<Search::Xapian::PerlMatchSpy>

=cut
