package Search::Xapian::Database;

use 5.006;
use strict;
use warnings;
use Carp;

use Search::Xapian::Enquire;

require DynaLoader;

our @ISA = qw(DynaLoader);

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

# Preloaded methods go here.

use overload '='  => sub { $_[0]->clone() },
	     'fallback' => 1;

sub enquire {
  my $self = shift;
  my $enquire = Search::Xapian::Enquire->new( $self );
  if( @_ ) {
    $enquire->set_query( @_ );
  }
  return $enquire;
}


sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new2( $self );
  bless $copy, $class;
  return $copy;
}

sub new() {
  my $class = shift;
  my $database;
  my $invalid_args;
  if( scalar(@_) == 1 ) {
    my $arg = shift;
    my $arg_class = ref( $arg );
    if( !$arg_class ) {
      $database = new1( $arg );
    } elsif( $arg_class eq $class ) {
      $database = new2( $arg );
    } else {
      $invalid_args = 1;
    }
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(\$file), $class->new(\$database)" );
    exit;
  }
  bless $database, $class;
  return $database;
}

1;

__END__

=head1 NAME

Search::Xapian::Database - Search database object

=head1 DESCRIPTION

This class represents a Xapian database for searching. See
L<Search::Xapian::WritableDatabase> for an object suitable for indexing.
To perform searches, this class works with the L<Search::Xapian::Query>
object.

=head1 METHODS

=over 4

=item new <database>

Class constructor. Can either take a path to an existing database
or another database class as the first parameter

=item clone

Return a clone of this class.

=item add_database

Add an existing database (or group of databases) to those accessed by this
object.

=item reopen

This re-opens the database(s) to the latest available version(s). It can be
used either to make sure the latest results are returned, or to recover from
a Xapian::DatabaseModifiedError.

=item close

Close the database. This also implies a commit() unless a transaction is in
progress.

=item enquire [<query>]

Returns a new L<Search::Xapian::Enquire> object. Any extra
parameters are passed to set_query.

=item get_doccount

Returns the number of document indexed in this database.

=item get_lastdocid

Returns the id of the last used document.

=item get_doclength <doc_id>

Returns the length of a given document.

=item get_document <doc_id>

Returns a L<Search::Xapian::Document> object for the given document.

=item get_avlength

Get the average length of the documents in the database.

=item get_termfreq <term>

Get the number of documents in the database indexed by a given term.

=item term_exists <term>

returns true if this term exists in the database, or false otherwise.

=item get_description

return a description of this object.

=item get_spelling_suggestion

returns a suggested spelling correction.

=item allterms_begin [<prefix>]

Returns a L<Search::Xapian::TermIterator> iterating over the termlist for the
the entire database.  If the optional prefix argument is non-empty, only
terms starting with that string are returned.

=item allterms_end [<prefix>]

Returns a L<Search::Xapian::TermIterator> pointing to the end of the
termlist corresponding to allterms_begin.

=item termlist_begin <docid>

Returns a L<Search::Xapian::TermIterator> pointing to the start of the
termlist for a given document.

=item termlist_end <docid>

Returns a L<Search::Xapian::TermIterator> pointing to the end of the
termlist for a given document.

=item positionlist_begin <docid> <term>

Returns a L<Search::Xapian::PositionIterator> pointing to the
start of the position list for a given term in the given document.

=item positionlist_end <docid> <term>

Returns a L<Search::Xapian::PositionIterator> pointing to the end
of the position list for a given term in the given document.

=item postlist_begin <term>

Returns a L<Search::Xapian::PostingIterator> pointing to the
start of the posting list for a given term.

=item postlist_end <term>

Returns a L<Search::Xapian::PostingIterator> pointing to the
end of the posting list for a given term.

=item keep_alive

Send a "keep-alive" to remote databases to stop them timing out.

=item get_collection_freq <term>

Get the number of elements indexed by a certain term.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Enquire>,L<Search::Xapian::WritableDatabase>

=cut
