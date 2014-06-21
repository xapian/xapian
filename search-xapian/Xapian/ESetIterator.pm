package Search::Xapian::ESetIterator;

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
	     '--' => sub { $_[0]->dec() },
	     '='  => sub { $_[0]->clone() },
	     'eq' => sub { $_[0]->equal($_[1]) },
	     'ne' => sub { $_[0]->nequal($_[1]) },
	     '==' => sub { $_[0]->equal($_[1]) },
	     '!=' => sub { $_[0]->nequal($_[1]) },
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
