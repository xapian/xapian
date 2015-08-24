package Search::Xapian::WritableDatabase;

=head1 NAME

Search::Xapian::WritableDatabase - writable database object

=head1 DESCRIPTION

This class represents a Xapian database for indexing. It's a subclass of
L<Search::Xapian::Database>, which is used for searching.

=head1 METHODS

=over 4

=item new <database> or new <path> <mode>

Class constructor. Takes either a database object, or a path and one
of DB_OPEN, DB_CREATE, DB_CREATE_OR_OPEN or DB_CREATE_OR_OVERWRITE.
These are exported by L<Search::Xapian> with the 'db' option.

=item clone

Return a clone of this class.

=item flush

Flush to disk any modifications made to the database.

For efficiency reasons, when performing multiple updates to a database it is
best (indeed, almost essential) to make as many modifications as memory will
permit in a single pass through the database. To ensure this, Xapian batches
up modifications.

Flush may be called at any time to ensure that the modifications which have
been made are written to disk: if the flush succeeds, all the preceding
modifications will have been written to disk.

If any of the modifications fail, an exception will be thrown and the database
will be left in a state in which each separate addition, replacement or
deletion operation has either been fully performed or not performed at all:
it is then up to the application to work out which operations need to be
repeated.

Beware of calling flush too frequently: this will have a severe performance
cost.

Note that flush need not be called explicitly: it will be called automatically
when the database is closed, or when a sufficient number of modifications
have been made.

=item add_document <document>

Add a new document to the database.

This method adds the specified document to the database, returning a newly
allocated document ID.

Note that this does not mean the document will immediately appear in the
database; see flush() for more details.

As with all database modification operations, the effect is atomic: the
document will either be fully added, or the document fails to be added and
an exception is thrown (possibly at a later time when flush is called or the
database is closed).

=item delete_document <doc_id>

Delete a document from the database. This method removes the document with
the specified document ID from the database.

Note that this does not mean the document will immediately disappear from
the database; see flush() for more details.

As with all database modification operations, the effect is atomic: the
document will either be fully removed, or the document fails to be removed
and an exception is thrown (possibly at a later time when flush is called or
the database is closed).

=item delete_document_by_term <term>

Delete any documents indexed by a term from the database. This method removes
any documents indexed by the specified term from the database.

The intended use is to allow UIDs from another system to easily be mapped to
terms in Xapian, although this method probably has other uses.

=item replace_document <doc_id> <document>

eplace a given document in the database.

This method replaces the document with the specified document ID. Note that
this does not mean the document will immediately change in the database; see
flush() for more details.

As with all database modification operations, the effect is atomic: the
document will either be fully replaced, or the document fails to be replaced
and an exception is thrown (possibly at a later time when flush is called or
the database is closed).

=item replace_document_by_term <unique_term> <document>

Replace any documents matching an unique term.

This method replaces any documents indexed by the specified term with the
specified document. If any documents are indexed by the term, the lowest
document ID will be used for the document, otherwise a new document ID
will be generated as for add_document.

The intended use is to allow UIDs from another system to easily be mapped
to terms in Xapian, although this method probably has other uses.

Note that this does not mean the document(s) will immediately change in the
database; see flush() for more details.

As with all database modification operations, the effect is atomic: the
document(s) will either be fully replaced, or the document(s) fail to be
replaced and an exception is thrown (possibly at a later time when flush is
called or the database is closed).

=item reopen

Re-open the database. makes sure you have a fresh db handle.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Enquire>,L<Search::Xapian::Database>

=cut
1;
