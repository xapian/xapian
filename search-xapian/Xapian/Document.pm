package Search::Xapian::Document;

use 5.006;
use strict;
use warnings;
use Carp;

require Exporter;
require DynaLoader;

our @ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our @EXPORT_OK = ( );

our @EXPORT = qw( );

=head1 NAME

Search::Xapian::Document - Search database object

=head1 DESCRIPTION

This class represents a Xapian database for searching. See 
L<Xapian::WriteableDatabase> for an object suitable for indexing.
To perform searches, this class works with the L<Search::Xapian::Query>
object.


=head1 METHODS

=over 4

=cut

=item new

Class constructor. Can either take a path to an existing datbase
or another database class as the first parameter

=item clone

Return a clone of this class.

=item get_value (value_no)

Returns the value by the assigned number.

=item add_value <value_no> <value>

Set a value by value number.

=item remove_value <value_no>

Removes the value with the assigned number.

=item clear_values

Clear all set values.

=item get_data

Return all document data.

=item set_data <data>

Set all document data. This can be anything you like.

=item add_posting <term> <position> [<weight>]

Adds a term at the given position. weight defaults to 1.

=item remove_posting <term> <position> [<weight]

Removes a term from the given position. weight defaults to 1.

=item add_term <term> [<weight>]

Adds a term without positional information. weight defaults to 1.

=item remove_term <term>

Removes a term without positional information.

=item clear_terms

Remove all terms from the document.

=item termlist_count 

Returns number of terms in the document.

=item termlist_begin

Iterator for the terms in this document. Returns a
L<Search::Xapian::TermIterator>. 

=item termlist_end

Equivalent end iterator for termlist_begin().  Returns a
L<Search::Xapian::TermIterator>. 

=item values_count

Return number of defined values for this document.

=item values_begin

Return a L<Xapian::Search::ValueIterator> pointing at the start of the
values in this document.

=item values_end

Return a L<Xapian::Search::ValueIterator> pointing at the end of the
values in this document.

=item get_description

Document description (for introspection)

=cut

# Preloaded methods go here.

use overload '='  => sub { $_[0]->clone() },
             'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new2( $self );
  bless $copy, $class;
  return $copy;
}
sub new() {
  my $class = shift;
  my $document;
  my $invalid_args;
  if( scalar(@_) == 0 ) {
    $document = new1();
  } elsif( scalar(@_) == 1 and ref( $_[1] ) eq $class ) {
    $document = new2(@_);
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(), $class->new(\$document)" );
    exit;
  }
  bless $document, $class;
  return $document;
}

1;

=back

=head1 SEE ALSO

L<Search::Xapian::Database>

=cut
