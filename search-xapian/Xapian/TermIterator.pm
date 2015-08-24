package Search::Xapian::TermIterator;

use 5.006;
use strict;
use warnings;
use Carp;

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
	     '""' => sub { $_[0]->get_termname() },
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

Search::Xapian::TermIterator - Iterate over sets of terms.

=head1 DESCRIPTION

This object represents a stream of terms. It overloads C<++> for
advancing the iterator, or you can explicitly call the C<inc> method.
This class also overloads C<eq>, C<ne>, C<==>, C<!=>, and C<"">
(stringification).

=head1 METHODS

=over 4

=item new

Constructor. Defaults to a uninitialized iterator.

=item clone

=item inc

Advance the iterator by one. (Called implictly by C<++> overloading)

=item skip_to <tname>

Skip the iterator to term tname, or the first term after tname if tname
isn't in the list of terms being iterated.

=item get_termname

Get the name of the current term.

=item get_wdf

Return the wdf of the current term (if meaningful).

=item get_termfreq

Return the term frequency of the current term (if meaningful).

=item positionlist_begin

Return L<Search::Xapian::PositionIterator> pointing to start of positionlist for current term.

=item positionlist_end

Return L<Search::Xapian::PositionIterator> pointing to end of positionlist for current term.

=item get_description

Returns a string describing this object.

=item equal <termiterator>

Checks if a termiterator is the same as this termiterator. Also overloaded as
the C<eq> and C<!=> operators.

=item nequal <termiterator>

Checks if a termiterator is different from this termiterator. Also overloaded
as the C<ne> and C<!=> operators.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Document>

=cut

