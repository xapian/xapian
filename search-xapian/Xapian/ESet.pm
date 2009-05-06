package Search::Xapian::ESet;

use 5.006;
use strict;
use warnings;
use Carp;

use Tie::Array;
require DynaLoader;

our @ISA = qw(Tie::Array DynaLoader);

# Preloaded methods go here.

# In a new thread, copy objects of this class to unblessed, undef values.
sub CLONE_SKIP { 1 }

use overload '='  => sub { $_[0]->clone() },
             'fallback' => 1;

sub new() {
  my $class = shift;
  my $mset;
  my $invalid_args;
  if( scalar(@_) == 0 ) {
    $mset = new1();
  } elsif( scalar(@_) == 1 and ref( $_[1] ) eq $class ) {
    $mset = new2(@_);
  } else {
    $invalid_args = 1;
  }
  if( $invalid_args ) {
    Carp::carp( "USAGE: $class->new(), $class->new(\$eset)" );
    exit;
  }
  bless $mset, $class;
  return $mset;
}

sub items {
  my $self = shift;
  my @array;
  tie( @array, 'Search::Xapian::ESet', $self );
  return @array;
}

sub TIEARRAY {
  my $class = shift;
  my $eset = shift;
  return bless $eset, $class;
}

1;
