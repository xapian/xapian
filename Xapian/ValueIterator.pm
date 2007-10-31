package Search::Xapian::ValueIterator;

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
             '""' => sub { $_[0]->get_value() },
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

Search::Xapian::ValueIterator - Iterate over value slots in a document.

=head1 DESCRIPTION

This object represents a stream of document values. It overloads C<++> for
advancing the iterator, or you can explicitly call the C<inc> method.
This class also overloads C<eq>, C<ne>, C<==>, C<!=>, and C<"">
(stringification).

=head1 METHODS

=over 4

=item new 

Constructor. Defaults to a uninitialized iterator.

=item clone

=item inc

Advance the iterator by one. (Called implictly by C<++> overloading )

=item get_valueno

Return the number of the value slot at the current position.

=item get_value

Return the string in the value slot at current position.  Also overloaded as
the C<""> operator.

=item get_description

Returns a string describing this object. 

=item equal <valueiterator>

Checks if a valueiterator is the same as this valueiterator. Also overloaded as
the C<eq> and C<!=> operators.

=item nequal <valueiterator>

Checks if a valueiterator is different from this valueiterator. Also overloaded
as the C<ne> and C<!=> operators.

=back

=head1 SEE ALSO

L<Search::Xapian>,L<Search::Xapian::Document>

=cut
