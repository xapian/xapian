package Search::Xapian::PostingIterator;

use 5.006;
use strict;
use warnings;
use Carp;
use Scalar::Util 'blessed';

require DynaLoader;

our @ISA = qw(DynaLoader);

# Preloaded methods go here.

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

use overload '++' => sub { $_[0]->inc() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
	     '""' => sub { $_[0]->get_description() },
	     '0+' => sub { $_[0]->get_docid() },
	     'fallback' => 1;

sub clone() {
  my $self = shift;
  my $class = ref( $self );
  my $copy = new2( $self );
  bless $copy, $class;
  return $copy;
}

sub equal() {
  my ($self, $other) = @_;
  if( blessed($other) && $other->isa('Search::Xapian::PostingIterator') ) {
    $self->equal1($other);
  } else {
    ($self+0) == ($other+0);
  }
}

sub nequal() {
  my ($self, $other) = @_;
  if( blessed($other) && $other->isa('Search::Xapian::PostingIterator') ) {
    $self->nequal1($other);
  } else {
    ($self+0) != ($other+0);
  }
}


sub new() {
  my $class = shift;
  my $iterator;
  my $invalid_args;
  if( scalar(@_) == 0 ) {
    $iterator = new1();
  } elsif( scalar(@_) == 1 and ref( $_[1] ) eq $class ) {
    $iterator = new2(@_);
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(), $class->new(\$iterator)" );
    exit;
  }
  bless $iterator, $class;
  return $iterator;
}

1;

__END__

=head1 NAME

Search::Xapian::PostingIterator - Iterate over the list of documents indexed
by a term.

=head1 DESCRIPTION

This iterator represents a stream of documents indexed by a term. It overloads
C<++> for advancing the iterator, or you can explicitly call the C<inc> method.
This class also overloads C<eq>, C<ne>, C<==>, C<!=>, and C<"">
(stringification).

=head1 METHODS

=over 4

=item new

Constructor. Defaults to an uninitialized iterator.

=item clone

=item inc

Advance the iterator by one. (Called implictly by C<++> overloading).

=item skip_to <tname>

Skip the iterator to term tname, or the first term after tname if tname
isn't in the list of terms being iterated.

=item get_docid

Get the unique id of this document.

=item get_wdf

Return the wdf of the current term (if meaningful).

=item positionlist_begin

Return L<Search::Xapian::PositionIterator> pointing to start of positionlist for current term.

=item positionlist_end

Return L<Search::Xapian::PositionIterator> pointing to end of positionlist for current term.

=item get_doclength

Get the length of the document at the current position in the postlist.

This information may be stored in the postlist, in which case this lookup
should be extremely fast (indeed, not require further disk access). If the
information is not present in the postlist, it will be retrieved from the
database, at a greater performance cost.

=item equal <term>

Checks if a term is the same as this term. Also overloaded to the C<eq>
and C<==> operators.

=item nequal <term>

Checks if a term is different from this term. Also overloaded to the C<ne>
and C<!=> operators.

=item get_description

Return a description of this object.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Database>

=cut
